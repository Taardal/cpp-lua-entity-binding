function Camera:onCreate()
    print("Camera:onCreate")
    print("self.entityId .. " .. self.entityId)
    self.someNewIndex = "foobar123"
    print("self.someNewIndex .. " .. self.someNewIndex)
end

function Camera:onUpdate()
    print("Camera:onUpdate [" .. self.entityId .. "]")

    if (Input.isKeyPressed("Esc")) then
        print("OMG OMG ESC IS PRESSED")
    end

    if (Input.isKeyPressed("Enter")) then
        print("enter is not pressed sadface")
    end
end