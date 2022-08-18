function Camera:onCreate()
    print("Camera:onCreate")
    print("self.entityId .. " .. self.entityId)
    self.someNewIndex = "foobar123"
    print("self.someNewIndex .. " .. self.someNewIndex)
end

function Camera:onUpdate()
    print("Camera:onUpdate [" .. self.entityId .. "]")
end