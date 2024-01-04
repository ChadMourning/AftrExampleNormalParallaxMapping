#include "GLViewNormalMapping.h"

#include "WorldList.h" //This is where we place all of our WOs
#include "ManagerOpenGLState.h" //We can change OpenGL State attributes with this
#include "Axes.h" //We can set Axes to on/off with this
#include "PhysicsEngineODE.h"

//Different WO used by this module
#include "WO.h"
#include "WOStatic.h"
#include "WOStaticPlane.h"
#include "WOStaticTrimesh.h"
#include "WOTrimesh.h"
#include "WOHumanCyborg.h"
#include "WOHumanCal3DPaladin.h"
#include "WOWayPointSpherical.h"
#include "WOLight.h"
#include "WOSkyBox.h"
#include "WOCar1970sBeater.h"
#include "Camera.h"
#include "CameraStandard.h"
#include "CameraChaseActorSmooth.h"
#include "CameraChaseActorAbsNormal.h"
#include "CameraChaseActorRelNormal.h"
#include "Model.h"
#include "ModelDataShared.h"
#include "ModelMesh.h"
#include "ModelMeshDataShared.h"
#include "ModelMeshSkin.h"
#include "WONVStaticPlane.h"
#include "WONVPhysX.h"
#include "WONVDynSphere.h"
#include "WOImGui.h" //GUI Demos also need to #include "AftrImGuiIncludes.h"
#include "AftrImGuiIncludes.h"
#include "AftrGLRendererBase.h"
#include "GLSLShaderDefaultGL32.h"
#include "GLSLShaderDefaultGL32NormalMapping.h"

using namespace Aftr;

GLViewNormalMapping* GLViewNormalMapping::New( const std::vector< std::string >& args )
{
   GLViewNormalMapping* glv = new GLViewNormalMapping( args );
   glv->init( Aftr::GRAVITY, Vector( 0, 0, -1.0f ), "aftr.conf", PHYSICS_ENGINE_TYPE::petODE );
   glv->onCreate();
   return glv;
}


GLViewNormalMapping::GLViewNormalMapping( const std::vector< std::string >& args ) : GLView( args )
{
   //Initialize any member variables that need to be used inside of LoadMap() here.
   //Note: At this point, the Managers are not yet initialized. The Engine initialization
   //occurs immediately after this method returns (see GLViewNormalMapping::New() for
   //reference). Then the engine invoke's GLView::loadMap() for this module.
   //After loadMap() returns, GLView::onCreate is finally invoked.

   //The order of execution of a module startup:
   //GLView::New() is invoked:
   //    calls GLView::init()
   //       calls GLView::loadMap() (as well as initializing the engine's Managers)
   //    calls GLView::onCreate()

   //GLViewNormalMapping::onCreate() is invoked after this module's LoadMap() is completed.
}


void GLViewNormalMapping::onCreate()
{
   //GLViewNormalMapping::onCreate() is invoked after this module's LoadMap() is completed.
   //At this point, all the managers are initialized. That is, the engine is fully initialized.

   if( this->pe != NULL )
   {
      //optionally, change gravity direction and magnitude here
      //The user could load these values from the module's aftr.conf
      this->pe->setGravityNormalizedVector( Vector( 0,0,-1.0f ) );
      this->pe->setGravityScalar( Aftr::GRAVITY );
   }
   this->setActorChaseType( STANDARDEZNAV ); //Default is STANDARDEZNAV mode
   //this->setNumPhysicsStepsPerRender( 0 ); //pause physics engine on start up; will remain paused till set to 1
}


GLViewNormalMapping::~GLViewNormalMapping()
{
   //Implicitly calls GLView::~GLView()
}


void GLViewNormalMapping::updateWorld()
{
   GLView::updateWorld(); //Just call the parent's update world first.
                          //If you want to add additional functionality, do it after
                          //this call.
   float coef = 60;
   light->setPosition(Vector(100 * cos(ManagerSDLTime::getNumMainLoopIterations() / coef), 100 * sin(ManagerSDLTime::getNumMainLoopIterations() / coef), 100));
}


void GLViewNormalMapping::onResizeWindow( GLsizei width, GLsizei height )
{
   GLView::onResizeWindow( width, height ); //call parent's resize method.
}


void GLViewNormalMapping::onMouseDown( const SDL_MouseButtonEvent& e )
{
   GLView::onMouseDown( e );
}


void GLViewNormalMapping::onMouseUp( const SDL_MouseButtonEvent& e )
{
   GLView::onMouseUp( e );
}


void GLViewNormalMapping::onMouseMove( const SDL_MouseMotionEvent& e )
{
   GLView::onMouseMove( e );
}


void GLViewNormalMapping::onKeyDown( const SDL_KeyboardEvent& key )
{
   GLView::onKeyDown( key );
   if( key.keysym.sym == SDLK_0 )
      this->setNumPhysicsStepsPerRender( 1 );

   if( key.keysym.sym == SDLK_1 )
   {
      wall->getModel()->useNextSkin();
   }

   if (key.keysym.sym == SDLK_2)
   {
      wall->getModel()->usePreviousSkin();
   }
}


void GLViewNormalMapping::onKeyUp( const SDL_KeyboardEvent& key )
{
   GLView::onKeyUp( key );
}


void Aftr::GLViewNormalMapping::loadMap()
{
   this->worldLst = new WorldList(); //WorldList is a 'smart' vector that is used to store WO*'s
   this->actorLst = new WorldList();
   this->netLst = new WorldList();

   ManagerOpenGLState::GL_CLIPPING_PLANE = 1000.0;
   ManagerOpenGLState::GL_NEAR_PLANE = 0.1f;
   ManagerOpenGLState::enableFrustumCulling = false;
   Axes::isVisible = true;
   this->glRenderer->isUsingShadowMapping( false ); //set to TRUE to enable shadow mapping, must be using GL 3.2+

   this->cam->setPosition( 15,15,10 );

   std::string shinyRedPlasticCube( ManagerEnvironmentConfiguration::getSMM() + "/models/cube4x4x4redShinyPlastic_pp.wrl" );
   std::string wheeledCar( ManagerEnvironmentConfiguration::getSMM() + "/models/rcx_treads.wrl" );
   std::string grass( ManagerEnvironmentConfiguration::getSMM() + "/models/grassFloor400x400_pp.wrl" );
   std::string human( ManagerEnvironmentConfiguration::getSMM() + "/models/human_chest.wrl" );
   
   //SkyBox Textures readily available
   std::vector< std::string > skyBoxImageNames; //vector to store texture paths
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_water+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_dust+6.jpg" );
   skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_mountains+6.jpg" );


   {
      //Create a light
      float ga = 0.1f; //Global Ambient Light level for this module
      ManagerLight::setGlobalAmbientLight( aftrColor4f( ga, ga, ga, 1.0f ) );
      light = WOLight::New();
      light->isDirectionalLight( false );
      light->setPosition( Vector( 0, 0, 100 ) );
      //Set the light's display matrix such that it casts light in a direction parallel to the -z axis (ie, downwards as though it was "high noon")
      //for shadow mapping to work, this->glRenderer->isUsingShadowMapping( true ), must be invoked.
      light->getModel()->setDisplayMatrix( Mat4::rotateIdentityMat( { 0, 1, 0 }, 90.0f * Aftr::DEGtoRAD ) );
      light->setLabel( "Light" );
      worldLst->push_back( light );
   }

   {
      //Create the SkyBox
      WO* wo = WOSkyBox::New( skyBoxImageNames.at( 0 ), this->getCameraPtrPtr() );
      wo->setPosition( Vector( 0, 0, 0 ) );
      wo->setLabel( "Sky Box" );
      wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
      worldLst->push_back( wo );
   }
   
   { 
      //////Create the infinite grass plane (the floor)
      //WO* wo = WO::New( grass, Vector( 1, 1, 1 ), MESH_SHADING_TYPE::mstFLAT );
      //wo->setPosition( Vector( 0, 0, 0 ) );
      //wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
      //wo->upon_async_model_loaded( [wo]()
      //   {
      //      ModelMeshSkin& grassSkin = wo->getModel()->getModelDataShared()->getModelMeshes().at( 0 )->getSkins().at( 0 );
      //      grassSkin.getMultiTextureSet().at( 0 ).setTexRepeats( 5.0f );
      //      grassSkin.setAmbient( aftrColor4f( 0.4f, 0.4f, 0.4f, 1.0f ) ); //Color of object when it is not in any light
      //      grassSkin.setDiffuse( aftrColor4f( 1.0f, 1.0f, 1.0f, 1.0f ) ); //Diffuse color components (ie, matte shading color of this object)
      //      grassSkin.setSpecular( aftrColor4f( 0.4f, 0.4f, 0.4f, 1.0f ) ); //Specular color component (ie, how "shiney" it is)
      //      grassSkin.setSpecularCoefficient( 10 ); // How "sharp" are the specular highlights (bigger is sharper, 1000 is very sharp, 10 is very dull)
      //   } );
      //wo->setLabel( "Grass" );
      //worldLst->push_back( wo );
   }
   

   {
      //const std::string path = ManagerEnvironmentConfiguration::getSMM() + "models/WOStoneWallBump/";
      const std::string path = ManagerEnvironmentConfiguration::getSMM() + "models/WOBrickBoxBump/";
      //wall = WO::New( path + "WOStoneWallBump.wrl", Vector( 7, 7, 7 ), MESH_SHADING_TYPE::mstFLAT );
      wall = WO::New(path + "WOBrickBoxBump.3DS", Vector(7, 7, 7), MESH_SHADING_TYPE::mstFLAT);
      WO* wo = wall;
      wall->upon_async_model_loaded( [wo,path]()
         {
            //auto colorTex = ManagerTex::loadTexAsync( path + "stoneWall_COLOR.png" );
            //auto nrmTex = ManagerTex::loadTexAsync( path + "stoneWall_NRM.png" );
            //auto dispTex = ManagerTex::loadTexAsync( path + "stoneWall_DISP.png" );
            //auto specTex = ManagerTex::loadTexAsync( path + "stoneWall_SPEC.png" );
            auto colorTex = ManagerTex::loadTexAsync( path + "Common 5_COLOR.png" );
            auto nrmTex = ManagerTex::loadTexAsync( path + "Common 5_NRM.png" );
            auto dispTex = ManagerTex::loadTexAsync( path + "Common 5_DISP.png" );
            auto specTex = ManagerTex::loadTexAsync( path + "Common 5_SPEC.png" );
        
            ModelMeshSkin crazyBumpSkin;
            crazyBumpSkin.setMeshShadingType( MESH_SHADING_TYPE::mstFLAT );
            crazyBumpSkin.getMultiTextureSet().clear();
            crazyBumpSkin.getMultiTextureSet().push_back( std::move(*colorTex ) );
            crazyBumpSkin.getMultiTextureSet().push_back( std::move(*nrmTex ) );
            crazyBumpSkin.getMultiTextureSet().push_back( std::move(*specTex ) );

            GLSLShader* ptr = ManagerShader::getDefaultShaderCopy();
            
            crazyBumpSkin.setShader( GLSLShaderDefaultGL32NormalMapping::New() );
            wo->getModel()->getSkins().push_back( std::move( crazyBumpSkin ) );
           // wo->getModel()->useNextSkin();
         } );
      wo->setPosition( Vector( 70, 10, 10 ) );
      wo->setLabel( "Stone Wall" );
      worldLst->push_back( wo );
   }

   //{
   //   //Create the infinite grass plane that uses the Open Dynamics Engine (ODE)
   //   WO* wo = WOStatic::New( grass, Vector(1,1,1), MESH_SHADING_TYPE::mstFLAT );
   //   ((WOStatic*)wo)->setODEPrimType( ODE_PRIM_TYPE::PLANE );
   //   wo->setPosition( Vector(0,0,0) );
   //   wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
   //   wo->getModel()->getModelDataShared()->getModelMeshes().at(0)->getSkins().at(0).getMultiTextureSet().at(0)->setTextureRepeats( 5.0f );
   //   wo->setLabel( "Grass" );
   //   worldLst->push_back( wo );
   //}

   //{
   //   //Create the infinite grass plane that uses NVIDIAPhysX(the floor)
   //   WO* wo = WONVStaticPlane::New( grass, Vector( 1, 1, 1 ), MESH_SHADING_TYPE::mstFLAT );
   //   wo->setPosition( Vector( 0, 0, 0 ) );
   //   wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
   //   wo->getModel()->getModelDataShared()->getModelMeshes().at( 0 )->getSkins().at( 0 ).getMultiTextureSet().at( 0 )->setTextureRepeats( 5.0f );
   //   wo->setLabel( "Grass" );
   //   worldLst->push_back( wo );
   //}

   //{
   //   //Create the infinite grass plane (the floor)
   //   WO* wo = WONVPhysX::New( shinyRedPlasticCube, Vector( 1, 1, 1 ), MESH_SHADING_TYPE::mstFLAT );
   //   wo->setPosition( Vector( 0, 0, 50.0f ) );
   //   wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
   //   wo->setLabel( "Grass" );
   //   worldLst->push_back( wo );
   //}

   //{
   //   WO* wo = WONVPhysX::New( shinyRedPlasticCube, Vector( 1, 1, 1 ), MESH_SHADING_TYPE::mstFLAT );
   //   wo->setPosition( Vector( 0, 0.5f, 75.0f ) );
   //   wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
   //   wo->setLabel( "Grass" );
   //   worldLst->push_back( wo );
   //}

   //{
   //   WO* wo = WONVDynSphere::New( ManagerEnvironmentConfiguration::getVariableValue( "sharedmultimediapath" ) + "/models/sphereRp5.wrl", Vector( 1.0f, 1.0f, 1.0f ), mstSMOOTH );
   //   wo->setPosition( 0, 0, 100.0f );
   //   wo->setLabel( "Sphere" );
   //   this->worldLst->push_back( wo );
   //}

   //{
   //   WO* wo = WOHumanCal3DPaladin::New( Vector( .5, 1, 1 ), 100 );
   //   ((WOHumanCal3DPaladin*)wo)->rayIsDrawn = false; //hide the "leg ray"
   //   ((WOHumanCal3DPaladin*)wo)->isVisible = false; //hide the Bounding Shell
   //   wo->setPosition( Vector( 20, 20, 20 ) );
   //   wo->setLabel( "Paladin" );
   //   worldLst->push_back( wo );
   //   actorLst->push_back( wo );
   //   netLst->push_back( wo );
   //   this->setActor( wo );
   //}
   //
   //{
   //   WO* wo = WOHumanCyborg::New( Vector( .5, 1.25, 1 ), 100 );
   //   wo->setPosition( Vector( 20, 10, 20 ) );
   //   wo->isVisible = false; //hide the WOHuman's bounding box
   //   ((WOHuman*)wo)->rayIsDrawn = false; //show the 'leg' ray
   //   wo->setLabel( "Human Cyborg" );
   //   worldLst->push_back( wo );
   //   actorLst->push_back( wo ); //Push the WOHuman as an actor
   //   netLst->push_back( wo );
   //   this->setActor( wo ); //Start module where human is the actor
   //}

   //{
   //   //Create and insert the WOWheeledVehicle
   //   std::vector< std::string > wheels;
   //   std::string wheelStr( "../../../shared/mm/models/WOCar1970sBeaterTire.wrl" );
   //   wheels.push_back( wheelStr );
   //   wheels.push_back( wheelStr );
   //   wheels.push_back( wheelStr );
   //   wheels.push_back( wheelStr );
   //   WO* wo = WOCar1970sBeater::New( "../../../shared/mm/models/WOCar1970sBeater.wrl", wheels );
   //   wo->setPosition( Vector( 5, -15, 20 ) );
   //   wo->setLabel( "Car 1970s Beater" );
   //   ((WOODE*)wo)->mass = 200;
   //   worldLst->push_back( wo );
   //   actorLst->push_back( wo );
   //   this->setActor( wo );
   //   netLst->push_back( wo );
   //}
   
   //Make a Dear Im Gui instance via the WOImGui in the engine... This calls
   //the default Dear ImGui demo that shows all the features... To create your own,
   //inherit from WOImGui and override WOImGui::drawImGui_for_this_frame(...) (among any others you need).
   WOImGui* gui = WOImGui::New( nullptr );
   gui->setLabel( "My Gui" );
   gui->subscribe_drawImGuiWidget(
      [this, gui]() //this is a lambda, the capture clause is in [], the input argument list is in (), and the body is in {}
      {
         ImGui::Begin("GUI");
         if (ImGui::Button("Bump"))
            this->wall->getModel()->useSkinAtIndex(1);
         if (ImGui::Button("Matte"))
            this->wall->getModel()->useSkinAtIndex(0);
         ImGui::End();

         //ImGui::ShowDemoWindow(); //Displays the default ImGui demo from C:/repos/aburn/engine/src/imgui_implot/implot_demo.cpp
         //WOImGui::draw_AftrImGui_Demo( gui ); //Displays a small Aftr Demo from C:/repos/aburn/engine/src/aftr/WOImGui.cpp
         //ImPlot::ShowDemoWindow(); //Displays the ImPlot demo using ImGui from C:/repos/aburn/engine/src/imgui_implot/implot_demo.cpp
      } );
   this->worldLst->push_back( gui );

   createNormalMappingWayPoints();
}


void GLViewNormalMapping::createNormalMappingWayPoints()
{
   // Create a waypoint with a radius of 3, a frequency of 5 seconds, activated by GLView's camera, and is visible.
   WayPointParametersBase params(this);
   params.frequency = 5000;
   params.useCamera = true;
   params.visible = true;
   WOWayPointSpherical* wayPt = WOWayPointSpherical::New( params, 3 );
   wayPt->setPosition( Vector( 50, 0, 3 ) );
   worldLst->push_back( wayPt );
}
