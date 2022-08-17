Storytime = {}

Storytime.Player = { foo = "bar" }

function Storytime.Player:onCreate()
    print("foo " .. self.foo)
end

Storytime.Player:onCreate()
