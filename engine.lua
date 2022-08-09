Engine  = {}

local components = {
	player = {
		TransformComponent = {
			transform = {
				x = 1.1,
				y = 1.1,
			}
		},
		TagComponent = {
			tag = "Player"
		}
	},
}

function Engine.getComponent(entityId, componentType)
	local entityComponents = components[entityId]
	if entityComponents == nil then
		print("No components for entity [" .. entityId .. "]")
		return nil
	end
	local component = entityComponents[componentType]
	if component == nil then
		print("No component for type [" .. componentType .. "]")
		return nil
	end
	return component
end

