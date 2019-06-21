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
