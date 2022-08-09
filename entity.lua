require("component")

Entity = {}

function Entity:new(entity)
	--print("Entity:new")
	entity = entity or {}
	setmetatable(entity, self)
	self.__index = self
	return entity
end

function Entity:getComponent(componentType, entityId)
	--print("Entity:getComponent [" .. componentType .. "]")
	local component = { 
		entityId = entityId or self.entityId 
	}
	if componentType == ComponentType.TagComponent then
		return TagComponent:new(component)
	end
	if componentType == ComponentType.TransformComponent then
		return TransformComponent:new(component)
	end
	--print("Unknown component")
	return nil
end
