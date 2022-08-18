function Player:onCreate()
    print("Player:onCreate")
    print("self.entityId .. " .. self.entityId)
    print("self.type .. " .. self.type)
    print("self.transform .. " .. self.transform.x .. ", " .. self.transform.y)
    print("self.hasComponent('ScriptComponent') .. " .. tostring(self.hasComponent('ScriptComponent')))
    print("self.hasComponent('TransformComponent') .. " .. tostring(self.hasComponent('TransformComponent')))
    print("self.hasComponent('FooComponent') .. " .. tostring(self.hasComponent('FooComponent')))
end

function Player:onUpdate()
    print("Player:onUpdate [" .. self.entityId .. "]")
end

function Player:onDestroy()
    print("Player:onDestroy [" .. self.entityId .. "]")
end