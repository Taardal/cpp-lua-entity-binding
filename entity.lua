require("components")

Entity = {}

function Entity:new(entity)
	print("Entity:new")
	entity = entity or {}
	setmetatable(entity, self)
	self.__index = self
	return entity
end

function Entity:getComponent(componentType)
	print("Entity:getComponent [" .. componentType .. "]")
	local component = { 
		entityId = self.entityId 
	}
	if componentType == "TagComponent" then
		return TagComponent:new(component)
	end
	if componentType == "TransformComponent" then
		return TransformComponent:new(component)
	end
	print("Unknown component")
	return nil
end
