Entity = {}

function Entity:new(entity)
	print("Entity:new")
	entity = entity or {}
	setmetatable(entity, self)
	self.__index = self
	return entity
end

function Entity:onCreate(entityId)
	print("Entity:onCreate")
	self.entityId = entityId
end

function Entity:getComponent(componentType)
	print("Entity:getComponent")
	print(componentType)
	return Engine.getComponent(self.entityId, componentType)
end

function Entity:updateComponent(componentType, onUpdateComponent)
	print("Entity:updateComponent")
	local component = self:getComponent(componentType)
	onUpdateComponent(component)
end
