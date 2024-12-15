#include "slthpch.h"
#include "Memento.h"

namespace Sloth {

	//Memento Class
	Memento::Memento(Ref<Scene> scene){
		m_Scene = scene;
	}

	// Method to save previous state of cells on the map
	void Memento::SaveCell(std::vector<std::string>& cells, std::vector<state>& s) {
		cellChunks.push_back(cells);

		for (int i = 0; i < cells.size(); i++)
		{
			if (cellMap.count(cells[i]))
			{
				cellMap[cells[i]].push_back(s[i]);
			}
			else
			{
				std::vector<state> v;
				v.push_back(s[i]);
				
				cellMap.insert({ cells[i], v });
			}
		}

	}

	// Method to undo to the previous state of the map before last change
	void Memento::UndoCell(std::vector<std::vector<Entity>>& objects, std::vector<std::vector<glm::vec2>>& oCoords, glm::vec2 cellSize) {
		bool foundObject = false;
		//std::vector< std::string >cc = cellChunks.back();
		if (!cellChunks.empty())
		{
			for (int i = 0; i < cellChunks.back().size(); i++) // check the list of cells at the top of the stack
			{
				// Parce cell coords from a string to intergers
				std::string str = cellChunks.back()[i];
				std::string delimiter = ",";

				size_t pos = 0;
				int cellPosX = 0, cellPosY = 0; // create the int version of the current string cell value
				std::string token;
				pos = str.find(delimiter);
				token = str.substr(0, pos);
				cellPosX = stof(token);
	
				token = str.substr(pos + 1, std::string::npos);
				cellPosY = stof(token);
				
				state s = cellMap[cellChunks.back()[i]].back(); // grab cell state

				for (int o = 0; o < objects[s.layer].size(); o++) //change states of object in cell to previous states
				{
					// convert objects location to cell value    e.g. for Cell size (2.56, 1.28): (2.56, 1.28) -> (1,1)  or (5.12, 5.12) -> (2, 4)
					float snapPosX = objects[s.layer][o].GetComponent<TransformComponent>().Translation.x;
					float snapPosY = objects[s.layer][o].GetComponent<TransformComponent>().Translation.y;
					
					float xCellIndex = (snapPosX / cellSize.x);
					float yCellIndex = (snapPosY / cellSize.y);

					float value = (int)(xCellIndex * 100 + .5);
					float xCellRound = (float)value / 100;

					value = (int)(yCellIndex * 100 + .5);
					float yCellRound = (float)value / 100;

					int xCellInt = xCellRound * 2;
					int yCellInt = yCellRound * 2;

					//Catch edge case that causes rounding negative numbers to be off by 1
					if (snapPosX < -0.001)
						xCellInt--;
					if (snapPosY < -0.001)
						yCellInt--;


					float x = objects[s.layer][o].GetComponent<TransformComponent>().Translation.x;
					float y = objects[s.layer][o].GetComponent<TransformComponent>().Translation.y;
					if (xCellInt == cellPosX)
					{
						if (yCellInt == cellPosY)
						{
							foundObject = true;
							// Recreate previous state of object if it wasn't an empty tile
							if (!s.isEmpty)
							{
								//Add all components and attributes to object of it's previous state
								objects[s.layer][o].GetComponent<TransformComponent>() = s.transformC;
								if (s.hasSRC)
								{
									if (objects[s.layer][o].HasComponent<SpriteRendererComponent>())
									{
										if (!objects[s.layer][o].HasComponent<AnimatorComponent>())
											AssetManager::DeleteTexture(objects[s.layer][o].GetComponent<SpriteRendererComponent>().Texture.get()->GetPath());
										objects[s.layer][o].GetComponent<SpriteRendererComponent>() = s.spriteRendererC;
										AssetManager::LoadTexture(s.spriteRendererC.Texture.get()->GetPath());
									}
									else
										objects[s.layer][o].AddComponent<SpriteRendererComponent>(s.spriteRendererC);
								}
								else
								{
									if (objects[s.layer][o].HasComponent<SpriteRendererComponent>())
										objects[s.layer][o].RemoveComponent<SpriteRendererComponent>();
								}

								if (s.hasAC)
								{
									if (objects[s.layer][o].HasComponent<AnimatorComponent>())
									{
										objects[s.layer][o].GetComponent<AnimatorComponent>().animator.ClearAnimations();
										objects[s.layer][o].GetComponent<AnimatorComponent>().animator = s.animatorC.animator;
									}
									else
										objects[s.layer][o].AddComponent<AnimatorComponent>(s.animatorC);
								}
								else
								{
									if (objects[s.layer][o].HasComponent<AnimatorComponent>())
										objects[s.layer][o].RemoveComponent<AnimatorComponent>();
								}
							}
							else // Previous state was an empty tile
							{
								// Delete current object
								if (!objects[s.layer][o].HasComponent<AnimatorComponent>())
									if (objects[s.layer][o].HasComponent<SpriteRendererComponent>())
										AssetManager::DeleteTexture(objects[s.layer][o].GetComponent<SpriteRendererComponent>().Texture.get()->GetPath());

								//count++;
								m_Scene->DestroyEntity(objects[s.layer][o]);
								objects[s.layer].erase(objects[s.layer].begin() + o);
								oCoords[s.layer].erase(oCoords[s.layer].begin() + o);
								
							}
							break;
						}
					}
				}
				//Undo if current cell is empty and it is re-adding objects
				if (!foundObject)
				{
					Entity object = m_Scene->CreateEntity(s.name);
					object.GetComponent<TransformComponent>() = s.transformC;
					if (s.hasSRC)
						object.AddComponent<SpriteRendererComponent>(s.spriteRendererC);
					if (s.hasAC)
						object.AddComponent<AnimatorComponent>(s.animatorC);
					objects[s.layer].push_back(object);
					glm::vec2 pos = glm::vec2(s.transformC.Translation.x, s.transformC.Translation.y);
					oCoords[s.layer].push_back(pos);
				}

				cellMap[cellChunks.back()[i]].pop_back(); // delete current cell change as it is completely reverted now.
				if (cellMap[cellChunks.back()[i]].empty())
					cellMap.erase(cellChunks.back()[i]);
			}
			cellChunks.pop_back(); // delete list of cells to be changed as they are all completed now.
		}
		if (objects[0].size() > 0)
		{
			int x = 0;// temp if statement for debugging
		}
	}
}
