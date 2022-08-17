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

print("===========================")
print("===========================")
print("===========================")
print("===========================")
print("===========================")

Player = Entity.new()
p = Player.new()
print("p.entityId " .. p.entityId)

function p.onUpdate()
    print("onUpdate")
end

p.onUpdate()

p = nil
collectgarbage()

print("Lua END")