local serverAddr = "cyb1.net"
local serverPort = 1847
local clientName = ...
local resX, resY = 80, 25

local encode
(function()
  local encMap = {
    ['nil'] = function(x)
      return '!'
    end,
    number = function(x)
      return '#' .. x .. '@'
    end,
    string = function(x)
      return '@' .. string.gsub(x, '@', '@.') .. '@~'
    end,
    boolean = function(x)
      if x then return '+' else return '-' end
    end,
    table = function(x)
      local result = '='
      for k, v in pairs(x) do
        result = result .. encode(k) .. encode(v)
      end
      return result .. '!'
    end
  }
  function encode(x)
    return encMap[type(x)](x)
  end
end)()

local function decode(cb)
  local s, sTable
  local function sNumber()
    local p, data = s, ''
    function s(x)
      local pos = string.find(x, '@')
      if pos then
        s = p
        s(tonumber(data .. string.sub(x, 1, pos - 1)), string.sub(x, pos + 1))
      else
        data = data .. x
      end
    end
  end
  local function sString()
    local p, data, escape = s, '', false
    function s(x)
      for i = 1, #x do
        local now = string.sub(x, i, i)
        if escape then
          if now == '.' then
            data = data .. '@'
            escape = false
          elseif now == '~' then
            s = p
            return s(data, string.sub(x, i + 1))
          else
            error('unknown escape: ' .. now)
          end
        elseif now == '@' then
          escape = true
        else
          data = data .. now
        end
      end
    end
  end
  local function sRoot()
    local p = s
    function s(x)
      if #x > 0 then
        s = p
        local tag, rem = string.sub(x, 1, 1), string.sub(x, 2)
        if tag == '!' then
          s(nil, rem)
        elseif tag == '#' then
          sNumber()
          s(rem)
        elseif tag == '@' then
          sString()
          s(rem)
        elseif tag == '+' then
          s(true, rem)
        elseif tag == '-' then
          s(false, rem)
        elseif tag == '=' then
          sTable()
          s(rem)
        else
          error('invalid tag: ' .. tag)
        end
      end
    end
  end
  function sTable()
    local p, data, key = s, {}
    function s(x, rem)
      if key == nil then
        if x == nil then
          s = p
          s(data, rem)
        else
          key = x
          sRoot()
          s(rem)
        end
      else
        data[key] = x
        key = nil
        sRoot()
        s(rem)
      end
    end
    sRoot()
  end
  function s(x, rem)
    cb(x)
    sRoot()
    s(rem)
  end
  sRoot()
  return function(x)
    s(x)
  end
end

local function resolve(short)
  for addr in component.list(short) do
    return addr
  end
  for addr in component.list() do
    if string.lower(string.sub(addr, 1, #short)) == string.lower(short) then
      return addr
    end
  end
end

local gpu = resolve("gpu")
if gpu then
  gpu = component.proxy(gpu)
  gpu.setResolution(resX, resY)
  gpu.setDepth(gpu.maxDepth())
  gpu.setBackground(0)
end
local function print(p)
  if gpu then
    gpu.copy(1, 1, resX, resY, 0, -1)
    gpu.fill(1, resY, resX, resY, ' ')
    gpu.setForeground(p.color)
    gpu.set(1, resY, p.text)
  end
  if p.beep then
    computer.beep(p.beep)
  end
end

local dbAddr, db = resolve("database")
if dbAddr then
  db = component.proxy(dbAddr)
end

local invCache = {}
local function getInv(invName)
  if not invCache[invName] then
    invCache[invName] = component.proxy(resolve(invName))
  end
  return invCache[invName]
end

local inet = component.proxy(resolve("internet"))
while true do
  print{text = "Connecting to " .. clientName .. "@" .. serverAddr .. ":" .. serverPort, color = 0xFFFF00}
  local socket = inet.connect(serverAddr, serverPort)
  local timeout = computer.uptime() + 3
  while socket and not socket.finishConnect() do
    if computer.uptime() > timeout then
      socket.close()
      socket = nil
      break
    end
  end
  if socket then
    local writeBuffer = encode(clientName)
    print{text = "Connected", color = 0x00FF00, beep = 440}
    local onRead = decode(function(p)
      for _, p in ipairs(p) do
        local inv, result
        if p.inv then inv = getInv(p.inv) end
        if p.op == "print" then
          print(p)
        elseif p.op == "list" then
          local stacks = inv.getAllStacks(p.side)
          result = {}
          for slot = 1, stacks.count() do
            local item = stacks()
            if item and item.name and item.size > 0 then
              result[slot] = item
            else
              result[slot] = ''
            end
          end
        elseif p.op == "listME" then
          result = {}
          for _, item in ipairs(inv.getItemsInNetwork()) do
            if item and item.name and item.size > 0 then
              table.insert(result, item)
            end
          end
        elseif p.op == "listXN" then
          local pos, stacks = {x = p.x, y = p.y, z = p.z}
          if p.side < 0 then
            stacks = inv.getItems(pos)
          else
            stacks = inv.getItems(pos, p.side)
          end
          result = {}
          for slot = 1, stacks.n do
            local item = stacks[slot]
            if item and item.name and item.size > 0 then
              result[slot] = item
            else
              result[slot] = ''
            end
          end
        elseif p.op == "xferME" then
          local me = getInv(p.me)
          db.clear(1)
          me.store(p.filter, dbAddr, 1, 1)
          me.setInterfaceConfiguration(1, dbAddr, 1, p.size)
          inv.transferItem(table.unpack(p.args))
          me.setInterfaceConfiguration(1)
        elseif p.op == "call" then
          -- transferItem (OC): fromSide, toSide, [size, [fromSlot, [toSlot]]]
          -- transferItem (XN): fromPos, fromSlot, size, toPos, [fromSide, [toSide]]
          result = {inv[p.fn](table.unpack(p.args))}
        else
          error("invalid op")
        end
        writeBuffer = writeBuffer .. encode(result)
      end
    end)
    while true do
      if #writeBuffer > 0 then
        local n = socket.write(writeBuffer)
        if not n then
          print{text = "Connection closed (write)", color = 0xFF0000, beep = 880}
          socket.close()
          break
        elseif n > 0 then
          writeBuffer = string.sub(writeBuffer, n + 1)
        end
      end
      local data = socket.read()
      if data then
        onRead(string.char(table.unpack(data)))
      else
        print{text = "Connection closed (read)", color = 0xFF0000, beep = 880}
        socket.close()
        break
      end
    end
  else
    print{text = "Failed to connect", color = 0xFF0000, beep = 880}
  end
end
