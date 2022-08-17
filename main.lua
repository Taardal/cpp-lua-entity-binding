print("Hello World from Lua")

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