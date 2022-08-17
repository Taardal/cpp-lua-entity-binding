print("Hello World from Lua")

function Player:onCreate()
    print("onCreate")
end

function Player:onUpdate()
    print("onUpdate")
    print("self.entityId .. " .. self.entityId)
    print("self.type .. " .. self.type)
    print("self.transform .. " .. self.transform.x .. ", " .. self.transform.y)
    print("self.hasComponent('ScriptComponent') .. " .. tostring(self.hasComponent('ScriptComponent')))
    print("self.hasComponent('TransformComponent') .. " .. tostring(self.hasComponent('TransformComponent')))
    print("self.hasComponent('FooComponent') .. " .. tostring(self.hasComponent('FooComponent')))
end

print("Lua END")