local entities = {}

function createEntity(entity)
	log("Creating entity [" .. entity.entityId .. "]")
	entities[entity.entityId] = entity
end

function printEntities()
	log("Printing entities")
	for entityId, entity in pairs(entities) do
		log("   " .. entityId)
	end
end
