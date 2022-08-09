require("entity")

Player = Entity:new()

function Player:onCreate(entityId)
	self.entityId = entityId
	self.tagComponent = self:getComponent("TagComponent")
	self.transformComponent = self:getComponent("TransformComponent")
end

function Player:onUpdate()
	print("Player:onUpdate")
	print("-- ID [" .. self.entityId .. "]")
	print("-- Tag [" .. self.tagComponent:getTag() .. "]")
	print("-- Transform [" .. self.transformComponent:getTransform().x .. ", " .. self.transformComponent:getTransform().y .. "]")
end

