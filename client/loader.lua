load((function()
  local content = ""
  for chunk in component.invoke(component.list("internet")(), "request", "https://leu-235.com/oc-scripts/client.lua").read do
    content = content .. chunk
  end
  return content
end)())("leu-235.com", 1847, "clientName", 80, 25)
