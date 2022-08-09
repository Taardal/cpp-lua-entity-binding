Player = Entity:new()

function Player:onCreate(entityId)
	self.entityId = entityId
end

function Player:onUpdate()
	print("player.onUpdate")

	local transformComponent = self:getComponent("TransformComponent")
	print(transformComponent.transform.x)
	print(transformComponent.transform.y)

	self:updateComponent("TransformComponent", function(component)
		component.transform.x = 12313
		component.transform.y = 99999
	end)

	local transformComponent = self:getComponent("TransformComponent")
	print(transformComponent.transform.x)
	print(transformComponent.transform.y)
end

