load((function()
  local content = ""
  for chunk in component.invoke(component.list("internet")(), "request", "https://cyb1.net/oc-scripts/client.lua").read do
    content = content .. chunk
  end
  return content
end)())("cyb1.net", 1847, "clientName", 80, 25)
