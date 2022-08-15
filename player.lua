require("entity")
require("component")
require("log")

Player = Entity:new()

function Player:onCreate()
	log("Player:onCreate")
	log("-- ID [" .. self.entityId .. "]")
--	self.tagComponent = self:getComponent(ComponentType.TagComponent)
--	self.transformComponent = self:getComponent(ComponentType.TransformComponent)
end

function Player:onUpdate()
	print("Player:onUpdate")
	print("-- ID [" .. self.entityId .. "]")
--	print("-- Tag [" .. self.tagComponent:getTag() .. "]")
--	print("-- Transform [" .. self.transformComponent:getTransform().x .. ", " .. self.transformComponent:getTransform().y .. "]")
end

