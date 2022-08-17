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
    print("Player:onUpdate")
end

function Camera:onCreate()
    print("Camera:onCreate")
    print("self.entityId .. " .. self.entityId)
    print("self.type .. " .. self.type)
    print("self.transform .. " .. self.transform.x .. ", " .. self.transform.y)
    print("self.hasComponent('ScriptComponent') .. " .. tostring(self.hasComponent('ScriptComponent')))
    print("self.hasComponent('TransformComponent') .. " .. tostring(self.hasComponent('TransformComponent')))
    print("self.hasComponent('FooComponent') .. " .. tostring(self.hasComponent('FooComponent')))
end

function Camera:onUpdate()
    print("Camera:onUpdate")
end