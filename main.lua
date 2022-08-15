require("log")
require("player")

local entities = {}

function printEntities()
	log("Printing entities")
	for entityId, entity in pairs(entities) do
		local entityExists = entity ~= nil
		log("   " .. entityId .. " --> " .. tostring(entityExists))
	end
end

function createEntity(entity)
	log("Creating entity [" .. entity.entityId .. "]")
	entities[entity.entityId] = entity
end

function initEntity(entityId)
	log("Initializing entity [" .. entityId .. "]")
	local entity = entities[entityId]
	entity:onCreate()
end

function updateEntity(entityId)
	log("Updating entity [" .. entityId .. "]")
	local entity = entities[entityId]
	entity:onUpdate()
end
