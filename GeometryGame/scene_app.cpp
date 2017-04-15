#include "scene_app.h"
#include <system/platform.h>
#include <graphics/sprite_renderer.h>
#include <graphics/font.h>
#include <system/debug_log.h>
#include <graphics/renderer_3d.h>
#include <graphics/mesh.h>
#include <maths/math_utils.h>
#include <input/sony_controller_input_manager.h>
#include <graphics/sprite.h>
#include "load_texture.h"
#include <input/touch_input_manager.h>
#include <input/input_manager.h>
#include <graphics/scene.h>
#include <animation/skeleton.h>
#include <animation/animation.h>
#include <input/keyboard.h>
#include <math.h>

SceneApp::SceneApp(gef::Platform& platform) :
	Application(platform),
	sprite_renderer_(NULL),
	renderer_3d_(NULL),
	primitive_builder_(NULL),
	input_manager_(NULL),
	audio_manager_(NULL),
	font_(NULL),
	world_(NULL),
	player_body_(NULL),
	sfx_id_(-1),
	sfx_voice_id_(-1),
	button_icon_cross(NULL),
	button_icon_square(NULL),
	button_icon_circle(NULL),
	is_paused(false),
	color("RED"),
	sound_volume_(1.0)
{
}

void SceneApp::Init()
{
	sprite_renderer_ = gef::SpriteRenderer::Create(platform_);
	InitFont();

	// initialise input manager
	input_manager_ = gef::InputManager::Create(platform_);

	// initialise audio manager
	audio_manager_ = gef::AudioManager::Create();

	//set the initianal state
	game_state_ = FRONTEND;
	FrontendInit();
}

void SceneApp::CleanUp()
{
	delete audio_manager_;
	audio_manager_ = NULL;

	delete input_manager_;
	input_manager_ = NULL;

	CleanUpFont();

	delete sprite_renderer_;
	sprite_renderer_ = NULL;
}

bool SceneApp::Update(float frame_time)
{
	fps_ = 1.0f / frame_time;


	input_manager_->Update();

	switch (game_state_)
	{
		case FRONTEND:
		{
			FrontendUpdate(frame_time);
		}
		break;
			
		case PLAY_GAME:
		{
			GameUpdate(frame_time);
		}
		break;
		case GAME_OPTIONS:
		{
			GameOptionsUpdate(frame_time);
		}
		break;
		case FINISH_SCREEN:
		{
			FinishUpdate(frame_time);
		}
		break;
		case PAUSE_SCREEN:
		{
			PausescreenUpdate(frame_time);
		}
	}
	return true;
}




void SceneApp::Render()
{
	switch (game_state_)
		{
		case FRONTEND:
		{
			FrontendRender();
		}
		break;

		case PLAY_GAME:
		{
			GameRender();
		}
		break;
		case GAME_OPTIONS:
		{
			GameOptionsRender();
		}
		break;
		case FINISH_SCREEN:
		{
			FinishRender();
		}
		break;
		case PAUSE_SCREEN:
		{
			PausescreenRender();
		}
		break;
	}
}

void SceneApp::InitPlayer()
{
	// setup the mesh for the player
	player_.set_mesh(primitive_builder_->GetDefaultCubeMesh());

	// create a physics body for the player
	b2BodyDef player_body_def;
	player_body_def.type = b2_dynamicBody;
	player_body_def.position = b2Vec2(0.0f, 4.0f);

	player_body_ = world_->CreateBody(&player_body_def);

	// create the shape for the player
	b2PolygonShape player_shape;
	player_shape.SetAsBox(0.5f, 0.5f);

	// create the fixture
	b2FixtureDef player_fixture_def;
	player_fixture_def.shape = &player_shape;
	player_fixture_def.density = 1.0f;

	// create the fixture on the rigid body
	player_body_->CreateFixture(&player_fixture_def);

	// update visuals from simulation data
	player_.UpdateFromSimulation(player_body_);

	// create a connection between the rigid body and GameObject
	player_body_->SetUserData(&player_);
}

void SceneApp::InitGround()
{
	// ground dimensions
	gef::Vector4 ground_half_dimensions(70.0f, 0.5f, 0.5f);
	gef::Vector4 ground_half_dimensions2(100.0f, 0.5f, 0.5f);

	// setup the mesh for the ground
	ground_mesh_ = primitive_builder_->CreateBoxMesh(ground_half_dimensions);
	ground_.set_mesh(ground_mesh_);

	ground_mesh_2 = primitive_builder_->CreateBoxMesh(ground_half_dimensions2);
	ground_2.set_mesh(ground_mesh_2);

	// create a physics body
	b2BodyDef body_def;
	body_def.type = b2_staticBody;
	body_def.position = b2Vec2(60.0f, 0.0f);

	b2BodyDef body_def2;
	body_def2.type = b2_staticBody;
	body_def2.position = b2Vec2(250.0f, -3.0f);

	//create the bodies
	ground_body_ = world_->CreateBody(&body_def);
	ground_body_2 = world_->CreateBody(&body_def2);

	// create the shape
	b2PolygonShape shape;
	shape.SetAsBox(ground_half_dimensions.x(), ground_half_dimensions.y());

	b2PolygonShape shape2;
	shape2.SetAsBox(ground_half_dimensions2.x(), ground_half_dimensions2.y());

	// create the fixture
	b2FixtureDef fixture_def;
	fixture_def.shape = &shape;

	b2FixtureDef fixture_def2;
	fixture_def2.shape = &shape2;

	// create the fixture on the rigid body
	ground_body_->CreateFixture(&fixture_def);
	ground_body_2->CreateFixture(&fixture_def2);

	// update visuals from simulation data
	ground_.UpdateFromSimulation(ground_body_);
	ground_2.UpdateFromSimulation(ground_body_2);
}

void SceneApp::InitPlatforms()
{
	// ground dimensions
	gef::Vector4 plat1_dimensions(0.5f, 0.5f, 0.5f);
	gef::Vector4 plat2_dimensions(0.5f, 1.0f, 0.5f);
	gef::Vector4 plat3_dimensions(0.5f, 0.5f, 0.5f);
	gef::Vector4 plat4_dimensions(4.0f, 1.5f, 0.5f);
	gef::Vector4 plat5_dimensions(0.5f, 2.0f, 0.5f);
	gef::Vector4 plat6_dimensions(0.5f, 1.5f, 0.5f);
	gef::Vector4 plat7_dimensions(0.5f, 1.0f, 0.5f);
	gef::Vector4 plat8_dimensions(10.0f, 1.0f, 0.5f);
	gef::Vector4 plat9_dimensions(5.0f, 1.0f, 0.5f);
	gef::Vector4 plat10_dimensions(1.0f, 1.5f, 0.5f);

	// setup the mesh for the ground
	plat1_mesh_ = primitive_builder_->CreateBoxMesh(plat1_dimensions);
	plat1.set_mesh(plat1_mesh_);

	plat2_mesh_ = primitive_builder_->CreateBoxMesh(plat2_dimensions);
	plat2.set_mesh(plat2_mesh_);

	plat3_mesh_ = primitive_builder_->CreateBoxMesh(plat3_dimensions);
	plat3.set_mesh(plat3_mesh_);

	plat4_mesh_ = primitive_builder_->CreateBoxMesh(plat4_dimensions);
	plat4.set_mesh(plat4_mesh_);

	plat5_mesh_ = primitive_builder_->CreateBoxMesh(plat5_dimensions);
	plat5.set_mesh(plat5_mesh_);

	plat6_mesh_ = primitive_builder_->CreateBoxMesh(plat6_dimensions);
	plat6.set_mesh(plat6_mesh_);

	plat7_mesh_ = primitive_builder_->CreateBoxMesh(plat7_dimensions);
	plat7.set_mesh(plat7_mesh_);

	plat8_mesh_ = primitive_builder_->CreateBoxMesh(plat8_dimensions);
	plat8.set_mesh(plat8_mesh_);

	plat9_mesh_ = primitive_builder_->CreateBoxMesh(plat9_dimensions);
	plat9.set_mesh(plat9_mesh_);

	plat10_mesh_ = primitive_builder_->CreateBoxMesh(plat10_dimensions);
	plat10.set_mesh(plat10_mesh_);

	// create a physics body
	b2BodyDef body_def_plat1;
	body_def_plat1.type = b2_staticBody;
	body_def_plat1.position = b2Vec2(8.0f, 1.0f);

	b2BodyDef body_def_plat2;
	body_def_plat2.type = b2_staticBody;
	body_def_plat2.position = b2Vec2(18.0f, 1.0f);

	b2BodyDef body_def_plat3;
	body_def_plat3.type = b2_staticBody;
	body_def_plat3.position = b2Vec2(25.0f, 2.5f);

	b2BodyDef body_def_plat4;
	body_def_plat4.type = b2_staticBody;
	body_def_plat4.position = b2Vec2(40.0f, 1.0f);

	b2BodyDef body_def_plat5;
	body_def_plat5.type = b2_staticBody;
	body_def_plat5.position = b2Vec2(50.0f, 2.5f);

	b2BodyDef body_def_plat6;
	body_def_plat6.type = b2_staticBody;
	body_def_plat6.position = b2Vec2(56.0f, 3.5f);

	b2BodyDef body_def_plat7;
	body_def_plat7.type = b2_staticBody;
	body_def_plat7.position = b2Vec2(65.0f, 1.25f);

	b2BodyDef body_def_plat8;
	body_def_plat8.type = b2_staticBody;
	body_def_plat8.position = b2Vec2(75.0f, 1.25f);

	b2BodyDef body_def_plat9;
	body_def_plat9.type = b2_staticBody;
	body_def_plat9.position = b2Vec2(100.0f, 1.25f);

	b2BodyDef body_def_plat10;
	body_def_plat10.type = b2_staticBody;
	body_def_plat10.position = b2Vec2(105.0f, 2.0f);

	//create bodies
	plat1_body_ = world_->CreateBody(&body_def_plat1);
	plat2_body_ = world_->CreateBody(&body_def_plat2);
	plat3_body_ = world_->CreateBody(&body_def_plat3);
	plat4_body_ = world_->CreateBody(&body_def_plat4);
	plat5_body_ = world_->CreateBody(&body_def_plat5);
	plat6_body_ = world_->CreateBody(&body_def_plat6);
	plat7_body_ = world_->CreateBody(&body_def_plat7);
	plat8_body_ = world_->CreateBody(&body_def_plat8);
	plat9_body_ = world_->CreateBody(&body_def_plat9);
	plat10_body_ = world_->CreateBody(&body_def_plat10);

	// create the shape
	b2PolygonShape shape_plat1;
	shape_plat1.SetAsBox(plat1_dimensions.x(), plat1_dimensions.y());

	b2PolygonShape shape_plat2;
	shape_plat2.SetAsBox(plat2_dimensions.x(), plat2_dimensions.y());

	b2PolygonShape shape_plat3;
	shape_plat3.SetAsBox(plat3_dimensions.x(), plat3_dimensions.y());

	b2PolygonShape shape_plat4;
	shape_plat4.SetAsBox(plat4_dimensions.x(), plat4_dimensions.y());

	b2PolygonShape shape_plat5;
	shape_plat5.SetAsBox(plat5_dimensions.x(), plat5_dimensions.y());

	b2PolygonShape shape_plat6;
	shape_plat6.SetAsBox(plat6_dimensions.x(), plat6_dimensions.y());

	b2PolygonShape shape_plat7;
	shape_plat7.SetAsBox(plat7_dimensions.x(), plat7_dimensions.y());

	b2PolygonShape shape_plat8;
	shape_plat8.SetAsBox(plat8_dimensions.x(), plat8_dimensions.y());

	b2PolygonShape shape_plat9;
	shape_plat9.SetAsBox(plat9_dimensions.x(), plat9_dimensions.y());

	b2PolygonShape shape_plat10;
	shape_plat10.SetAsBox(plat10_dimensions.x(), plat10_dimensions.y());

	// create the fixture
	b2FixtureDef fixture_def_plat1;
	fixture_def_plat1.shape = &shape_plat1;

	b2FixtureDef fixture_def_plat2;
	fixture_def_plat2.shape = &shape_plat2;

	b2FixtureDef fixture_def_plat3;
	fixture_def_plat3.shape = &shape_plat3;

	b2FixtureDef fixture_def_plat4;
	fixture_def_plat4.shape = &shape_plat4;

	b2FixtureDef fixture_def_plat5;
	fixture_def_plat5.shape = &shape_plat5;

	b2FixtureDef fixture_def_plat6;
	fixture_def_plat6.shape = &shape_plat6;

	b2FixtureDef fixture_def_plat7;
	fixture_def_plat7.shape = &shape_plat7;

	b2FixtureDef fixture_def_plat8;
	fixture_def_plat8.shape = &shape_plat8;

	b2FixtureDef fixture_def_plat9;
	fixture_def_plat9.shape = &shape_plat9;

	b2FixtureDef fixture_def_plat10;
	fixture_def_plat10.shape = &shape_plat10;

	// create the fixture on the rigid body
	plat1_body_->CreateFixture(&fixture_def_plat1);
	plat2_body_->CreateFixture(&fixture_def_plat2);
	plat3_body_->CreateFixture(&fixture_def_plat3);
	plat4_body_->CreateFixture(&fixture_def_plat4);
	plat5_body_->CreateFixture(&fixture_def_plat5);
	plat6_body_->CreateFixture(&fixture_def_plat6);
	plat7_body_->CreateFixture(&fixture_def_plat7);
	plat8_body_->CreateFixture(&fixture_def_plat8);
	plat9_body_->CreateFixture(&fixture_def_plat9);
	plat10_body_->CreateFixture(&fixture_def_plat10);

	// update visuals from simulation data
	plat1.UpdateFromSimulation(plat1_body_);
	plat2.UpdateFromSimulation(plat2_body_);
	plat3.UpdateFromSimulation(plat3_body_);
	plat4.UpdateFromSimulation(plat4_body_);
	plat5.UpdateFromSimulation(plat5_body_);
	plat6.UpdateFromSimulation(plat6_body_);
	plat7.UpdateFromSimulation(plat7_body_);
	plat8.UpdateFromSimulation(plat8_body_);
	plat9.UpdateFromSimulation(plat9_body_);
	plat10.UpdateFromSimulation(plat10_body_);
}


void SceneApp::InitFont()
{
	font_ = new gef::Font(platform_);
	font_->Load("comic_sans");
}

void SceneApp::CleanUpFont()
{
	delete font_;
	font_ = NULL;
}

void SceneApp::DrawHUD()
{
	if(font_)
	{
		// display frame rate
		font_->RenderText(sprite_renderer_, gef::Vector4(850.0f, 510.0f, -0.9f), 1.0f, 0xffffffff, gef::TJ_LEFT, "FPS: %.1f", fps_);
	}
}

void SceneApp::SetupLights()
{
	// grab the data for the default shader used for rendering 3D geometry
	gef::Default3DShaderData& default_shader_data = renderer_3d_->default_shader_data();

	// set the ambient light
	default_shader_data.set_ambient_light_colour(gef::Colour(0.25f, 0.25f, 0.25f, 1.0f));

	// add a point light that is almost white, but with a blue tinge
	// the position of the light is set far away so it acts light a directional light
	gef::PointLight default_point_light;
	default_point_light.set_colour(gef::Colour(0.7f, 0.7f, 1.0f, 1.0f));
	default_point_light.set_position(gef::Vector4(-500.0f, 400.0f, 700.0f));
	default_shader_data.AddPointLight(default_point_light);
}

void SceneApp::UpdateSimulation(float frame_time)
{
	//gef::DebugOut("%.f \n", player_body_->GetLinearVelocity().x);
	//gef::DebugOut("%.11f \n", player_body_->GetPosition().y);
	player_body_->SetAngularVelocity(0);


	game_speed += 0.05;

	if (player_body_->GetLinearVelocity().x < 4) {
		player_body_->ApplyLinearImpulseToCenter(b2Vec2(0.5f, 0.0f), true);
	}

	else if (player_body_->GetLinearVelocity().x < 20) {
		player_body_->ApplyLinearImpulseToCenter(b2Vec2(0.04f, 0.0f), true);
	}

	camera_pos = player_body_->GetPosition().x;
	camera_pos = player_body_->GetPosition().x;
	// update physics world

	float32 timeStep = 1.0f / 60.0f;

	int32 velocityIterations = 6;
	int32 positionIterations = 2;

	world_->Step(timeStep, velocityIterations, positionIterations);

	// update object visuals from simulation data
	player_.UpdateFromSimulation(player_body_);

	// don't have to update the ground visuals as it is static

	// collision detection
	// get the head of the contact list
	b2Contact* contact = world_->GetContactList();
	// get contact count
	int contact_count = world_->GetContactCount();
	for (int contact_num = 0; contact_num<contact_count; ++contact_num)
	{
		if (contact->IsTouching())
		{
			// get the colliding bodies
			b2Body* bodyA = contact->GetFixtureA()->GetBody();
			b2Body* bodyB = contact->GetFixtureB()->GetBody();

			// DO COLLISION RESPONSE HERE
			Player* player = NULL;

			GameObject* gameObjectA = NULL;
			GameObject* gameObjectB = NULL;

			gameObjectA = (GameObject*)bodyA->GetUserData();
			gameObjectB = (GameObject*)bodyB->GetUserData();

			if (gameObjectA)
			{
				if (gameObjectA->type() == FINISH)
				{
					
				}
			}

			if (gameObjectB)
			{
				if (gameObjectB->type() == PLAYER)
				{
					player = (Player*)bodyB->GetUserData();
				}
			}

			if (player)
			{
				player->DecrementHealth();
			}
		}
		// Get next contact point
		contact = contact->GetNext();

		const gef::SonyController* controller = input_manager_->controller_input()->GetController(0);
		gef::Keyboard* keyboard = input_manager_->keyboard();

		if (controller->buttons_pressed() && gef_SONY_CTRL_CROSS || (keyboard->IsKeyPressed(gef::Keyboard::KC_X))) {
			player_body_->ApplyLinearImpulseToCenter(b2Vec2(0.0f, 7.0f), true);
		}
	}
}

void SceneApp::FrontendInit()
{
	button_icon_cross = CreateTextureFromPNG("playstation-cross-dark-icon.png", platform_);
	start_selected = true;
}

void SceneApp::FrontendRelease()
{
	delete button_icon_cross;
	button_icon_cross = NULL;
}

void SceneApp::FrontendUpdate(float frame_time)
{
	const gef::SonyController* controller = input_manager_->controller_input()->GetController(0);
	gef::Keyboard* keyboard = input_manager_->keyboard();

	if (controller->buttons_pressed() && gef_SONY_CTRL_CROSS || (keyboard->IsKeyPressed(gef::Keyboard::KC_X)))
	{	
		if (start_selected == true )
		{
			FrontendRelease();
			game_state_ = PLAY_GAME;
			GameInit();
		}
		else {
			FrontendRelease();
			game_state_ = GAME_OPTIONS;
			GameOptionsInit();
		}
	}




	if (controller->buttons_pressed() && gef_SONY_CTRL_DOWN && start_selected == true || (keyboard->IsKeyPressed(gef::Keyboard::KC_DOWN)) && start_selected == true)
	{
		start_selected = false;
	}

	if (controller->buttons_pressed() && gef_SONY_CTRL_UP & start_selected == false || (keyboard->IsKeyPressed(gef::Keyboard::KC_UP)) && start_selected == false)
	{
		start_selected = true;
	
	}
}

void SceneApp::FrontendRender()
{
	sprite_renderer_->Begin();

	if (start_selected == true) {
	// render selected START THE GAME
	font_->RenderText(
		sprite_renderer_,
		gef::Vector4(platform_.width()*0.5f, platform_.height()*0.5f - 56.0f, -0.99f),
		1.5f,
		0xffffffff,
		gef::TJ_CENTRE,
		"START THE GAME");

	// render unselected OPTIONS
	font_->RenderText(
		sprite_renderer_,
		gef::Vector4(platform_.width()*0.5f, platform_.height()*0.5f + 32.0f, -0.99f),
		1.0f,
		0xff000000,
		gef::TJ_CENTRE,
		"OPTIONS");
	}
	else{
			// render unselected START THE GAME
			font_->RenderText(
				sprite_renderer_,
				gef::Vector4(platform_.width()*0.5f, platform_.height()*0.5f - 56.0f, -0.99f),
				1.0f,
				0xff000000,
				gef::TJ_CENTRE,
				"START THE GAME");

			// render selected OPTIONS
			font_->RenderText(
				sprite_renderer_,
				gef::Vector4(platform_.width()*0.5f, platform_.height()*0.5f + 32.0f, -0.99f),
				1.5f,
				0xffffffff,
				gef::TJ_CENTRE,
				"OPTIONS");
		}


	DrawHUD();
	sprite_renderer_->End();
}

void SceneApp::GameInit()
{

	game_speed = 1;
	camera_pos = 0;

	// create the renderer for draw 3D geometry
	renderer_3d_ = gef::Renderer3D::Create(platform_);

	// initialise primitive builder to make create some 3D geometry easier
	primitive_builder_ = new PrimitiveBuilder(platform_);


	SetupLights();

	// initialise the physics world
	b2Vec2 gravity(0.0f, -9.81f);
	world_ = new b2World(gravity);

	InitPlayer();
	InitGround();
	InitPlatforms();

	// load audio assets
	if (audio_manager_)
	{
		// load a sound effect
		sfx_id_ = audio_manager_->LoadSample("box_collected.wav", platform_);

		// load in music
		audio_manager_->LoadMusic("music.wav", platform_);

		// play music
		audio_manager_->PlayMusic();
	}
}

void SceneApp::GameRelease()
{
	
	// unload audio resources
	if (audio_manager_)
	{
		audio_manager_->StopMusic();
		audio_manager_->UnloadAllSamples();
		sfx_id_ = -1;
		sfx_voice_id_ = -1;
	}

	// destroying the physics world also destroys all the objects within it
	delete ground_mesh_2;
	ground_mesh_2 = NULL;

	delete world_;
	world_ = NULL;

	delete ground_mesh_;
	ground_mesh_ = NULL;

	delete plat1_mesh_;
	plat1_mesh_ = NULL;

	delete plat2_mesh_;
	plat2_mesh_ = NULL;

	delete plat3_mesh_;
	plat3_mesh_ = NULL;

	delete plat4_mesh_;
	plat4_mesh_ = NULL;

	delete plat5_mesh_;
	plat5_mesh_ = NULL;

	delete plat6_mesh_;
	plat6_mesh_ = NULL;

	delete plat7_mesh_;
	plat7_mesh_ = NULL;

	delete plat8_mesh_;
	plat8_mesh_ = NULL;

	delete plat9_mesh_;
	plat9_mesh_ = NULL;

	delete plat10_mesh_;
	plat10_mesh_ = NULL;

	delete primitive_builder_;
	primitive_builder_ = NULL;

	delete renderer_3d_;
	renderer_3d_ = NULL;

}

void SceneApp::GameUpdate(float frame_time)
{
	gef::Keyboard* keyboard = input_manager_->keyboard();
	const gef::SonyController* controller = input_manager_->controller_input()->GetController(0);

	// trigger a sound effect

	UpdateSimulation(frame_time);

	if (controller->buttons_pressed() && gef_SONY_CTRL_START || (keyboard->IsKeyPressed(gef::Keyboard::KC_P)))
	{
		is_paused = true;
		game_state_ = PAUSE_SCREEN;
		PausescreenInit();
	}


}

void SceneApp::GameRender()
{
	// setup camera

	// projection
	float fov = gef::DegToRad(45.0f);
	float aspect_ratio = (float)platform_.width() / (float)platform_.height();
	gef::Matrix44 projection_matrix;
	projection_matrix = platform_.PerspectiveProjectionFov(fov, aspect_ratio, 0.1f, 100.0f);
	renderer_3d_->set_projection_matrix(projection_matrix);

	// view
	gef::Vector4 camera_eye(camera_pos, 2.0f, 15.0f);
	gef::Vector4 camera_lookat(camera_pos, 0.0f, 0.0f);
	gef::Vector4 camera_up(0.0f, 1.0f, 0.0f);
	gef::Matrix44 view_matrix;
	view_matrix.LookAt(camera_eye, camera_lookat, camera_up);
	renderer_3d_->set_view_matrix(view_matrix);


	// draw 3d geometry
	renderer_3d_->Begin();

	// draw ground
	renderer_3d_->DrawMesh(ground_);
	renderer_3d_->DrawMesh(ground_2);

	//draw platforms
	renderer_3d_->DrawMesh(plat1);
	renderer_3d_->DrawMesh(plat2);
	renderer_3d_->DrawMesh(plat3);
	renderer_3d_->DrawMesh(plat4);
	renderer_3d_->DrawMesh(plat5);
	renderer_3d_->DrawMesh(plat6);
	renderer_3d_->DrawMesh(plat7);
	renderer_3d_->DrawMesh(plat8);
	renderer_3d_->DrawMesh(plat9);
	renderer_3d_->DrawMesh(plat10);

	// draw player
	if(color == "RED"){
		renderer_3d_->set_override_material(&primitive_builder_->red_material());
	}
	if (color == "BLUE")
	{
		renderer_3d_->set_override_material(&primitive_builder_->blue_material());
	}
	if (color == "GREEN")
	{
		renderer_3d_->set_override_material(&primitive_builder_->green_material());
	}
	renderer_3d_->DrawMesh(player_);
	renderer_3d_->set_override_material(NULL);

	renderer_3d_->End();

	// start drawing sprites, but don't clear the frame buffer
	sprite_renderer_->Begin(false);
	DrawHUD();
	sprite_renderer_->End();
}

void SceneApp::GameOptionsInit()
{
	button_icon_circle = CreateTextureFromPNG("playstation-circle-dark-icon.png", platform_);
	sound_selected = true;
}

void SceneApp::GameOptionsRelease()
{
	delete button_icon_circle;
	button_icon_circle = NULL;
}

void SceneApp::GameOptionsUpdate(float frame_time)
{
	const gef::SonyController* controller = input_manager_->controller_input()->GetController(0);
	gef::Keyboard* keyboard = input_manager_->keyboard();

	if (controller->buttons_pressed() & gef_SONY_CTRL_CIRCLE || (keyboard->IsKeyPressed(gef::Keyboard::KC_B))) {
		if (is_paused == false)
		{
			GameOptionsRelease();
			game_state_ = FRONTEND;
			FrontendInit();
		}
		if (is_paused == true)
		{
			GameOptionsRelease();
			game_state_ = PAUSE_SCREEN;
			PausescreenInit();
		}
	}

	if (sound_selected == true) {
		if (sound_volume_ >= -0.5f & sound_volume_ <= 1.0f)
		{
			if (sound_volume_ < 1.0f) {
				if (controller->buttons_pressed() & gef_SONY_CTRL_RIGHT || (keyboard->IsKeyPressed(gef::Keyboard::KC_RIGHT)))
				{
					sound_volume_ = sound_volume_ + 0.1;
					GameOptionsRender();
				}
			}
			
			if (controller->buttons_pressed() & gef_SONY_CTRL_LEFT || (keyboard->IsKeyPressed(gef::Keyboard::KC_LEFT)))
			{
				if (sound_volume_ > 0.0f)
				{
					sound_volume_ = sound_volume_ - 0.1;
					GameOptionsRender();
		
				}
				

			}
		}

	}
	else {

		if (controller->buttons_pressed() && gef_SONY_CTRL_LEFT && color == "RED" || (keyboard->IsKeyPressed(gef::Keyboard::KC_LEFT)) && color == "RED")
		{
			color = "GREEN";
			GameOptionsRender();
		}
		else if (controller->buttons_pressed() && gef_SONY_CTRL_LEFT && color == "GREEN" || (keyboard->IsKeyPressed(gef::Keyboard::KC_LEFT)) && color == "GREEN")
		{
			color = "BLUE";
			GameOptionsRender();
		}
		else if (controller->buttons_pressed() && gef_SONY_CTRL_LEFT && color == "BLUE" || (keyboard->IsKeyPressed(gef::Keyboard::KC_LEFT)) && color == "BLUE")
		{
			color = "RED";
			GameOptionsRender();
		}

		if (controller->buttons_pressed() && gef_SONY_CTRL_RIGHT && color == "RED" || (keyboard->IsKeyPressed(gef::Keyboard::KC_RIGHT)) && color == "RED")
		{
			color = "BLUE";
			GameOptionsRender();
		}
		else if (controller->buttons_pressed() && gef_SONY_CTRL_RIGHT && color == "BLUE" || (keyboard->IsKeyPressed(gef::Keyboard::KC_RIGHT)) && color == "BLUE")
		{
			color = "GREEN";
			GameOptionsRender();
		}
		else if (controller->buttons_pressed() && gef_SONY_CTRL_RIGHT && color == "GREEN" || (keyboard->IsKeyPressed(gef::Keyboard::KC_RIGHT)) && color == "GREEN")
		{
			color = "RED";
			GameOptionsRender();
		}
	}

	if (controller->buttons_pressed() && gef_SONY_CTRL_DOWN && sound_selected == true || (keyboard->IsKeyPressed(gef::Keyboard::KC_DOWN)) && sound_selected == true)
	{
		sound_selected = false;
	}

	if (controller->buttons_pressed() && gef_SONY_CTRL_UP & sound_selected == false || (keyboard->IsKeyPressed(gef::Keyboard::KC_UP)) && sound_selected == false)
	{
		sound_selected = true;

	}
}

void SceneApp::GameOptionsRender()
{
	sprite_renderer_->Begin();
	if (sound_selected == true) {


		font_->RenderText(
			sprite_renderer_,
			gef::Vector4(platform_.width()*0.5f, platform_.height()*0.5f - 56.0f, -0.99f),
			1.5f,
			0xffffffff,
			gef::TJ_CENTRE,
			"SOUND : %.0f", fabs(sound_volume_ * 10));


		font_->RenderText(
			sprite_renderer_,
			gef::Vector4(platform_.width()*0.5f, platform_.height()*0.5f - 16.0f, -0.99f),
			1.0f,
			0xff000000,
			gef::TJ_CENTRE,
			"CUBE COLOR: %s", color);
	}
	else {
		font_->RenderText(
			sprite_renderer_,
			gef::Vector4(platform_.width()*0.5f, platform_.height()*0.5f - 56.0f, -0.99f),
			1.0f,
			0xff000000,
			gef::TJ_CENTRE,
			"SOUND : %.0f", sound_volume_ * 10);

		font_->RenderText(
			sprite_renderer_,
			gef::Vector4(platform_.width()*0.5f, platform_.height()*0.5f - 16.0f, -0.99f),
			1.5f,
			0xffffffff,
			gef::TJ_CENTRE,
			"CUBE COLOR: %s", color);
	}

	//render circle icon
	gef::Sprite button_back;
	button_back.set_texture(button_icon_circle);
	button_back.set_position(gef::Vector4(platform_.width()*0.5f + 200.0f, platform_.height()*0.5f + 170.0f, -0.99f));
	button_back.set_height(32.0f);
	button_back.set_width(32.0f);
	sprite_renderer_->DrawSprite(button_back);

	font_->RenderText(
		sprite_renderer_,
		gef::Vector4(platform_.width()*0.5f + 250.0f, platform_.height()*0.5f + 150.0f, -0.99f),
		1.0f,
		0xffffffff,
		gef::TJ_CENTRE,
		"BACK");

	sprite_renderer_->End();
}

void SceneApp::FinishInit()
{
}

void SceneApp::FinishRelease()
{
}

void SceneApp::FinishUpdate(float frame_time)
{
}

void SceneApp::FinishRender()
{
}

void SceneApp::PausescreenInit()
{
	continue_selected = true;
}

void SceneApp::PausescreenRelease()
{
}

void SceneApp::PausescreenUpdate(float frame_time)
{
	const gef::SonyController* controller = input_manager_->controller_input()->GetController(0);
	gef::Keyboard* keyboard = input_manager_->keyboard();

	if (controller->buttons_pressed() & gef_SONY_CTRL_DOWN || (keyboard->IsKeyPressed(gef::Keyboard::KC_DOWN)))
	{
		if (continue_selected == true)
		{
			continue_selected = false;
			options_selected = true;
		}
		else if (options_selected == true) {
			options_selected = false;
		}
	}

	else if (controller->buttons_pressed() & gef_SONY_CTRL_UP || (keyboard->IsKeyPressed(gef::Keyboard::KC_UP)))
	{
		if (options_selected == true)
		{
			options_selected = false;
			continue_selected = true;

		}
		if (options_selected == false && continue_selected == false)
		{
			options_selected = true;
		}
	}

	if (controller->buttons_pressed() & gef_SONY_CTRL_CROSS || (keyboard->IsKeyPressed(gef::Keyboard::KC_X)))
	{
		if (continue_selected == true)
		{
			game_state_ = PLAY_GAME;
			GameInit();
			is_paused = false;
		}
		else if (options_selected == true) {
			game_state_ = GAME_OPTIONS;
			GameOptionsInit();
		}
		else if (options_selected == false && continue_selected == false)
		{
			game_state_ = FRONTEND;
			FrontendInit();
			is_paused = false;
		}
	}

}
	


void SceneApp::PausescreenRender()
{

	sprite_renderer_->Begin();
	if (continue_selected == true) {


		font_->RenderText(
			sprite_renderer_,
			gef::Vector4(platform_.width()*0.5f, platform_.height()*0.5f - 56.0f, -0.99f),
			1.5f,
			0xffffffff,
			gef::TJ_CENTRE,
			"Continue");


		font_->RenderText(
			sprite_renderer_,
			gef::Vector4(platform_.width()*0.5f, platform_.height()*0.5f - 16.0f, -0.99f),
			1.0f,
			0xff000000,
			gef::TJ_CENTRE,
			"Options");

		font_->RenderText(
			sprite_renderer_,
			gef::Vector4(platform_.width()*0.5f, platform_.height()*0.5f + 14.0f, -0.99f),
			1.0f,
			0xff000000,
			gef::TJ_CENTRE,
			"Quit");

	}
	else if(options_selected == true) {

		font_->RenderText(
			sprite_renderer_,
			gef::Vector4(platform_.width()*0.5f, platform_.height()*0.5f - 56.0f, -0.99f),
			1.5f,
			0xff000000,
			gef::TJ_CENTRE,
			"Continue");

		font_->RenderText(
			sprite_renderer_,
			gef::Vector4(platform_.width()*0.5f, platform_.height()*0.5f - 16.0f, -0.99f),
			1.0f,
			0xffffffff,
			gef::TJ_CENTRE,
			"Options");

		font_->RenderText(
			sprite_renderer_,
			gef::Vector4(platform_.width()*0.5f, platform_.height()*0.5f + 14.0f, -0.99f),
			1.0f,
			0xff000000,
			gef::TJ_CENTRE,
			"Quit");
	}
	else if (options_selected == false && continue_selected == false)
	{
		font_->RenderText(
			sprite_renderer_,
			gef::Vector4(platform_.width()*0.5f, platform_.height()*0.5f - 56.0f, -0.99f),
			1.5f,
			0xff000000,
			gef::TJ_CENTRE,
			"Continue");

		font_->RenderText(
			sprite_renderer_,
			gef::Vector4(platform_.width()*0.5f, platform_.height()*0.5f - 16.0f, -0.99f),
			1.0f,
			0xff000000,
			gef::TJ_CENTRE,
			"Options");

		font_->RenderText(
			sprite_renderer_,
			gef::Vector4(platform_.width()*0.5f, platform_.height()*0.5f + 14.0f, -0.99f),
			1.0f,
			0xffffffff,
			gef::TJ_CENTRE,
			"Quit");
	}
	

	sprite_renderer_->End();
}
