print("Hello World from Lua")

e = Entity.new()
print("e.entityId " .. e.entityId)

e.entityId = "some_other_entityId"
print("e.entityId " .. e.entityId)

e = nil
collectgarbage()

print("Lua END")