print("Hello World from Lua")

Player = Entity.new()

function Player.onUpdate()
    print("onUpdate")
end

p = Player.new()
print("p.entityId " .. p.entityId)

p.onUpdate()

p = nil
collectgarbage()

print("Lua END")