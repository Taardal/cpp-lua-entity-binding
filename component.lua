require("engine")

--
-- COMPONENT TYPE
--
ComponentType = {
	TagComponent = "TagComponent",
	TransformComponent = "TransformComponent",
}

--
-- COMPONENT
--
Component = {}

function Component:new(component)
	--print("Component:new")
	local component = component or {}
	setmetatable(component, self)
	self.__index = self
	return component
end

--
-- TAG COMPONENT
--

TagComponent = Component:new()

function TagComponent:getTag()
	--print("TagComponent:getTag")
	return Engine.TagComponent_getTag(self.entityId)
end

--
-- TRANSFORM COMPONENT
--

TransformComponent = Component:new()

function TransformComponent:getTransform()
	--print("TransformComponent:getTransform")
	return Engine.TransformComponent_getTransform(self.entityId)
end

