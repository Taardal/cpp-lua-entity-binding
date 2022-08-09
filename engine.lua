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

function Engine.TagComponent_getTag(entityId)
	--print("Engine.TagComponent_getTag [" .. entityId .. "]")
	return components[entityId].TagComponent.tag	
end

function Engine.TransformComponent_getTransform(entityId)
	--print("Engine.TransformComponent_getTransform [" .. entityId .. "]")
	return components[entityId].TransformComponent.transform	
end
