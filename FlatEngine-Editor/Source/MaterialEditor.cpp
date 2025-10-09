#include "FlatEngine.h"
#include "FlatGui.h"
#include "Material.h"
#include "GraphicsPipeline.h"
#include "VulkanManager.h"

#include "imgui.h"


namespace FL = FlatEngine;

using Material = FL::Material;

namespace FlatGui
{
	std::string newMaterialFileName = "";
	bool b_openCreateMaterialModal = false;
	bool FG_b_showMaterialEditor = false;

	void RenderMaterialEditor()
	{
		FL::BeginWindow("Material Editor", FG_b_showMaterialEditor);
		// {		

		if (FL::F_selectedMaterialName.size() > 0)
		{
			std::shared_ptr<Material> currentMaterial = FL::F_VulkanManager->GetMaterial(FL::F_selectedMaterialName);
			std::map<std::string, std::shared_ptr<Material>>& materials = FL::F_VulkanManager->GetMaterials();

			ImGui::PushStyleColor(ImGuiCol_ChildBg, FL::GetColor("innerWindow"));
			ImGui::BeginChild("Material Selection", Vector2(0), FL::F_headerFlags);
			ImGui::PopStyleColor();
			// {

			FL::MoveScreenCursor(10, 5);
			ImGui::Text("Select material to edit:");
			FL::MoveScreenCursor(0, 5);

			if (materials.size() > 0)
			{
				FL::PushComboStyles();
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 144);
				if (ImGui::BeginCombo("##materials", FL::F_selectedMaterialName.c_str()))
				{
					for (std::map<std::string, std::shared_ptr<Material>>::iterator material = materials.begin(); material != materials.end(); material++)
					{						
						bool b_isSelected = (material->second->GetName() == FL::F_selectedMaterialName);
						ImGui::PushStyleColor(ImGuiCol_FrameBg, FL::GetColor("outerWindow"));
						if (ImGui::Selectable(material->second->GetName().c_str(), b_isSelected))
						{
							FL::F_selectedMaterialName = material->second->GetName();
						}
						if (b_isSelected)
						{
							ImGui::SetItemDefaultFocus();
						}
						ImGui::PopStyleColor();
					}
					ImGui::EndCombo();
				}
				FL::PopComboStyles();

				ImGui::SameLine();

				if (FL::F_MappingContexts.size() > 0)
				{
					if (FL::RenderButton("Save"))
					{
						FL::F_VulkanManager->SaveMaterial(currentMaterial);
						//currentMaterial->Init();
						//FL::F_VulkanManager->InitializeMaterials();
					}
				}
				ImGui::SameLine(0, 5);
				if (FL::RenderButton("New Material"))
				{
					b_openCreateMaterialModal = true;
				}
			}

			// }
			ImGui::EndChild(); // Material Selection


			FL::RenderSeparator(10, 10);


			ImGui::PushStyleColor(ImGuiCol_ChildBg, FL::GetColor("innerWindow"));
			ImGui::BeginChild("Edit Material", Vector2(0), FL::F_headerFlags);
			ImGui::PopStyleColor();
			// {

			if (currentMaterial != nullptr)
			{
				static std::string vertexInputText = "";
				static std::string fragmentInputText = "";
				vertexInputText = currentMaterial->GetVertexPath();
				fragmentInputText = currentMaterial->GetFragmentPath();
				uint32_t textureCount = currentMaterial->GetTextureCount();
				std::map<uint32_t, FL::TexturePipelineData>* texturesShaderData = currentMaterial->GetTexturesPipelineData();
				std::map<uint32_t, std::string> uboVec4Names = currentMaterial->GetUBOVec4Names();
				FL::TexturePipelineData textureData = FL::TexturePipelineData();
				static bool b_vertexSampler = false;
				static bool b_fragmentSampler = true;
				static bool b_combinedImageSampler = true;
				static bool b_depthImage = false;

				if (FL::RenderInput("##VertexShaderPathInput", "Vertex Shader Path", vertexInputText))
				{					
					FL::F_VulkanManager->SetMaterialVertexPath(currentMaterial->GetName(), vertexInputText);
				}

				if (FL::RenderInput("##FragmentShaderPathInput", "Fragment Shader Path", fragmentInputText))
				{					
					FL::F_VulkanManager->SetMaterialFragmentPath(currentMaterial->GetName(), fragmentInputText);
				}

				if (FL::RenderCheckbox("Vertex Sampler", b_vertexSampler))
				{
					b_vertexSampler = true;
					b_fragmentSampler = false;
				}
				ImGui::SameLine();
				if (FL::RenderCheckbox("Fragment Sampler", b_fragmentSampler))
				{
					b_vertexSampler = false;
					b_fragmentSampler = true;					
				}

				if (FL::RenderCheckbox("Combined Image Sampler", b_combinedImageSampler))
				{
					b_combinedImageSampler = true;
					b_depthImage = false;					
				}
				ImGui::SameLine();
				if (FL::RenderCheckbox("Depth Image", b_depthImage))
				{
					b_combinedImageSampler = false;
					b_depthImage = true;					
				}

				if (FL::RenderButton("Add Sampler"))
				{
					if (b_fragmentSampler)
					{
						textureData.shaderStage = VK_SHADER_STAGE_FRAGMENT_BIT;
					}
					else
					{
						textureData.shaderStage = VK_SHADER_STAGE_VERTEX_BIT;
					}
					if (b_combinedImageSampler)
					{
						textureData.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					}
					else
					{
						textureData.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
					}

					FL::F_VulkanManager->AddTextureToMaterial(currentMaterial->GetName(), textureCount, textureData);					
					FL::F_VulkanManager->SaveMaterial(currentMaterial);
				}

				for (std::map<uint32_t, FL::TexturePipelineData>::iterator iterator = texturesShaderData->begin(); iterator != texturesShaderData->end(); iterator++)
				{
					switch (iterator->second.shaderStage)
					{
					case VK_SHADER_STAGE_VERTEX_BIT:
					{
						std::string vertexString = "";
						if (iterator->second.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
						{
							vertexString = "Vertex Sampler : layout(binding = " + std::to_string(iterator->first + 1) + ") uniform sampler2D [name])";
						}
						else
						{
							vertexString = "Vertex Depth : layout(input_attachment_index = 0, binding = " + std::to_string(iterator->first + 1) + ") uniform subpassInput inputDepth;";							
						}
						ImGui::Text(vertexString.c_str());
						break;
					}

					case VK_SHADER_STAGE_FRAGMENT_BIT:
					{
						std::string fragmentString = "";
						if (iterator->second.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
						{
							fragmentString = "Fragment Sampler : layout(binding = " + std::to_string(iterator->first + 1) + ") uniform sampler2D [name])";
						}
						else
						{
							fragmentString = "Fragment Depth : layout(input_attachment_index = 0, binding = " + std::to_string(iterator->first + 1) + ") uniform subpassInput inputDepth;";
						}
						ImGui::Text(fragmentString.c_str());
						break;
					}
					default:
						break;
					}
				}
				if (FL::RenderButton("Remove Last Texture"))
				{					
					FL::F_VulkanManager->RemoveTextureFromMaterial(currentMaterial->GetName());
					FL::F_VulkanManager->SaveMaterial(currentMaterial);					
				}

				// Whenever we add a new property to a materials UBO, we should make sure to recreate the commandBuffers in the Models of the Meshes that use that material, taking into account the new Uniform Buffer Size
				ImGui::Text("Vec4s");

				static std::string vec4Name = "";

				FL::RenderInput("##NewVec4Name", "Name", vec4Name);
			
				if (FL::RenderButton("Add Vec4") && vec4Name != "")
				{
					if (currentMaterial->AddUBOVec4(vec4Name))
					{
						for (std::pair<long, Mesh> mesh : FL::GetMeshes())
						{
							if (mesh.second.GetMaterialName() == FL::F_selectedMaterialName)
							{
								mesh.second.SetUBOVec4(vec4Name, Vector4());
							}
						}
						vec4Name = "";

						//FL::F_VulkanManager->SaveMaterial(currentMaterial);						
					}
				}

				if (FL::RenderButton("Remove Last Vec4"))
				{
					FL::F_VulkanManager->RemoveUBOVec4FromMaterial(currentMaterial->GetName());	

					//FL::F_VulkanManager->SaveMaterial(currentMaterial);					
				}

				for (std::map<uint32_t, std::string>::iterator iter = uboVec4Names.begin(); iter != uboVec4Names.end(); iter++)
				{
					// TODO: Add editing of and adding of vec4 names here, then "refresh" the Meshes (emplace new std::pair<std::string, glm::vec4> in their m_uboVec4s members) that use this Material to account for the new vec4, or add a button to refresh it in the Mesh Component in inspector.
					// Currently the vec4s are global (the same for all Materials) because they and are created in constructor of Material and added to the Meshes m_uboVec4s in Mesh::SetMaterial() method. TODO: Make this dynamic
					std::string text = "vec4 vec4s[" + std::to_string(iter->first) + "]    Name: " + iter->second;
					ImGui::Text(text.c_str());
				}
			}

			// }
			ImGui::EndChild();
		}


		// Create new Mapping Context modal
		if (FL::RenderInputModal("Create New Material", "Enter a name for the new Material", newMaterialFileName, b_openCreateMaterialModal))
		{
			FL::F_VulkanManager->CreateNewMaterialFile(newMaterialFileName);
			FL::F_selectedMaterialName = newMaterialFileName;
			FG_b_showMaterialEditor = true;
		}

		// }
		FL::EndWindow();
	}
}