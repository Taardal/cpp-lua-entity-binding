print("Hello World from Lua")

e = Entity.new()
print("e.entityId " .. e.entityId)

e.entityId = "some_other_entityId"
print("e.entityId " .. e.entityId)

function e.onUpdate()
    print("onUpdate")
end

e.onUpdate()

e = nil
collectgarbage()

print("Lua END")