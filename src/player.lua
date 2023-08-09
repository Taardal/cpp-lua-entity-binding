function Player:onCreate()
    print("Player:onCreate")
    print("self.entityId .. " .. self.entityId)
    print("self.type .. " .. self.type)
    print("self.transform .. " .. self.transform.x .. ", " .. self.transform.y)
    print("self.hasComponent('scriptComponent') .. " .. tostring(self.hasComponent('scriptComponent')))
    print("self.hasComponent('transformComponent') .. " .. tostring(self.hasComponent('transformComponent')))
    print("self.hasComponent('FooComponent') .. " .. tostring(self.hasComponent('FooComponent')))
end

function Player:onUpdate()
    print("Player:onUpdate [" .. self.entityId .. "]")
end

function Player:onDestroy()
    print("Player:onDestroy [" .. self.entityId .. "]")
end