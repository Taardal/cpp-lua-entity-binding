print("Hello World from Lua")

function Player:onCreate()
    print("onCreate")
end

function Player:onUpdate()
    print("onUpdate")
    print("self.entityId .. " .. self.entityId)
end

print("Lua END")