/*
-----------------------------------------------------------------------------
Filename:    CartoonHairSimulation.cpp
-----------------------------------------------------------------------------


This source file is generated by the
   ___                   _              __    __ _                  _ 
  /___\__ _ _ __ ___    /_\  _ __  _ __/ / /\ \ (_)______ _ _ __ __| |
 //  // _` | '__/ _ \  //_\\| '_ \| '_ \ \/  \/ / |_  / _` | '__/ _` |
/ \_// (_| | | |  __/ /  _  \ |_) | |_) \  /\  /| |/ / (_| | | | (_| |
\___/ \__, |_|  \___| \_/ \_/ .__/| .__/ \/  \/ |_/___\__,_|_|  \__,_|
      |___/                 |_|   |_|                                 
      Ogre 1.8.x Application Wizard for VC10 (May 2012)
      https://bitbucket.org/jacmoe/ogreappwizards
-----------------------------------------------------------------------------
*/

#include "stdafx.h"

//#include "tinyxml2.h"

#include "CartoonHairSimulation.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#include "../res/resource.h"
#endif

//-------------------------------------------------------------------------------------
//taken from http://www.ogre3d.org/tikiwiki/tiki-index.php?page=RetrieveVertexData
//according to the terms listed on this webpage - the code is in the public domain
//this can be used to get the vertices from the mesh
void getMeshInformation(const Ogre::MeshPtr mesh,
                        size_t &vertex_count,
                        Ogre::Vector3* &vertices,
                        size_t &index_count,
                        unsigned long* &indices,
                        const Ogre::Vector3 &position,
                        const Ogre::Quaternion &orient,
                        const Ogre::Vector3 &scale)
{
    bool added_shared = false;
    size_t current_offset = 0;
    size_t shared_offset = 0;
    size_t next_offset = 0;
    size_t index_offset = 0;
 
    vertex_count = index_count = 0;
 
    // Calculate how many vertices and indices we're going to need
    for ( unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i)
    {
        Ogre::SubMesh* submesh = mesh->getSubMesh(i);
        // We only need to add the shared vertices once
        if(submesh->useSharedVertices)
        {
            if( !added_shared )
            {
                vertex_count += mesh->sharedVertexData->vertexCount;
                added_shared = true;
            }
        }
        else
        {
            vertex_count += submesh->vertexData->vertexCount;
        }
        // Add the indices
        index_count += submesh->indexData->indexCount;
    }
 
    // Allocate space for the vertices and indices
    vertices = new Ogre::Vector3[vertex_count];
    indices = new unsigned long[index_count];
 
    added_shared = false;
 
    // Run through the submeshes again, adding the data into the arrays
    for (unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i)
    {
        Ogre::SubMesh* submesh = mesh->getSubMesh(i);
 
        Ogre::VertexData* vertex_data = submesh->useSharedVertices ? mesh->sharedVertexData : submesh->vertexData;
 
        if ((!submesh->useSharedVertices) || (submesh->useSharedVertices && !added_shared))
        {
            if(submesh->useSharedVertices)
            {
                added_shared = true;
                shared_offset = current_offset;
            }
 
            const Ogre::VertexElement* posElem =
                vertex_data->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);
 
            Ogre::HardwareVertexBufferSharedPtr vbuf =
                vertex_data->vertexBufferBinding->getBuffer(posElem->getSource());
 
            unsigned char* vertex =
                static_cast<unsigned char*>(vbuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
 
            // There is _no_ baseVertexPointerToElement() which takes an Ogre::Real or a double
            //  as second argument. So make it float, to avoid trouble when Ogre::Real will
            //  be comiled/typedefed as double:
            //Ogre::Real* pReal;
            float* pReal;
 
            for( size_t j = 0; j < vertex_data->vertexCount; ++j, vertex += vbuf->getVertexSize())
            {
                posElem->baseVertexPointerToElement(vertex, &pReal);
                Ogre::Vector3 pt(pReal[0], pReal[1], pReal[2]);
                vertices[current_offset + j] = (orient * (pt * scale)) + position;
            }
 
            vbuf->unlock();
            next_offset += vertex_data->vertexCount;
        }
 
        Ogre::IndexData* index_data = submesh->indexData;
        size_t numTris = index_data->indexCount / 3;
        Ogre::HardwareIndexBufferSharedPtr ibuf = index_data->indexBuffer;
 
        bool use32bitindexes = (ibuf->getType() == Ogre::HardwareIndexBuffer::IT_32BIT);
 
        unsigned long* pLong = static_cast<unsigned long*>(ibuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
        unsigned short* pShort = reinterpret_cast<unsigned short*>(pLong);
 
        size_t offset = (submesh->useSharedVertices)? shared_offset : current_offset;
 
        if ( use32bitindexes )
        {
            for ( size_t k = 0; k < numTris*3; ++k)
            {
                indices[index_offset++] = pLong[k] + static_cast<unsigned long>(offset);
            }
        }
        else
        {
            for ( size_t k = 0; k < numTris*3; ++k)
            {
                indices[index_offset++] = static_cast<unsigned long>(pShort[k]) +
                                          static_cast<unsigned long>(offset);
            }
        }
 
        ibuf->unlock();
        current_offset = next_offset;
    }
}


CartoonHairSimulation::CartoonHairSimulation(void)
    : mRoot(0),
    mCamera(0),
    mSceneMgr(0),
    mWindow(0),
    mResourcesCfg(Ogre::StringUtil::BLANK),
    mPluginsCfg(Ogre::StringUtil::BLANK),
    mTrayMgr(0),
    mCameraMan(0),
    mDetailsPanel(0),
    mCursorWasVisible(false),
    mShutDown(false),
    mInputManager(0),
    mMouse(0),
    mKeyboard(0)
{
	mWorld = NULL;
	mSoftBodySolver = NULL;
	mConstraintSolver = NULL;
	mBroadphase = NULL;
	mDispatcher = NULL;
	mCollisionConfig = NULL;
	m_hairModel = NULL;
}

//-------------------------------------------------------------------------------------
CartoonHairSimulation::~CartoonHairSimulation(void)
{
    if (mTrayMgr) delete mTrayMgr;
    if (mCameraMan) delete mCameraMan;

    //Remove ourself as a Window listener
    Ogre::WindowEventUtilities::removeWindowEventListener(mWindow, this);
    windowClosed(mWindow);

	delete mRoot;

	if(m_hairModel)
	{
		delete m_hairModel;
	}

	//clean up physics
	if(mWorld)
	{
		delete mWorld;
		delete mSoftBodySolver;
		delete mConstraintSolver;
		delete mBroadphase;
		delete mDispatcher;
		delete mCollisionConfig;
	}

	//TO DO: add any clean up you need here
}

//-------------------------------------------------------------------------------------
bool CartoonHairSimulation::configure(void)
{
    // Show the configuration dialog and initialise the system
    // You can skip this and use root.restoreConfig() to load configuration
    // settings if you were sure there are valid ones saved in ogre.cfg
    if(mRoot->showConfigDialog())
    {
        // If returned true, user clicked OK so initialise
        // Here we choose to let the system create a default rendering window by passing 'true'
        mWindow = mRoot->initialise(true, "CartoonHairSimulation Render Window");

        // Let's add a nice window icon
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
        HWND hwnd;
        mWindow->getCustomAttribute("WINDOW", (void*)&hwnd);
        LONG iconID   = (LONG)LoadIcon( GetModuleHandle(0), MAKEINTRESOURCE(IDI_APPICON) );
        SetClassLong( hwnd, GCL_HICON, iconID );
#endif
        return true;
    }
    else
    {
        return false;
    }
}
//-------------------------------------------------------------------------------------
void CartoonHairSimulation::chooseSceneManager(void)
{
    // Get the SceneManager, in this case a generic one
    mSceneMgr = mRoot->createSceneManager(Ogre::ST_GENERIC);
}
//-------------------------------------------------------------------------------------
void CartoonHairSimulation::createCamera(void)
{
    // Create the camera
    mCamera = mSceneMgr->createCamera("PlayerCam");

    // Position it at 500 in Z direction
    mCamera->setPosition(Ogre::Vector3(0,0,2));
    // Look back along -Z
    mCamera->lookAt(Ogre::Vector3(0,0,-300));
    mCamera->setNearClipDistance(0.1);
	mCamera->setFarClipDistance(1000);

    mCameraMan = new OgreBites::SdkCameraMan(mCamera);   // create a default camera controller
	mCameraMan->setTopSpeed(4.0f);
}
//-------------------------------------------------------------------------------------
void CartoonHairSimulation::createFrameListener(void)
{
    Ogre::LogManager::getSingletonPtr()->logMessage("*** Initializing OIS ***");
    OIS::ParamList pl;
    size_t windowHnd = 0;
    std::ostringstream windowHndStr;

    mWindow->getCustomAttribute("WINDOW", &windowHnd);
    windowHndStr << windowHnd;
    pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));

    mInputManager = OIS::InputManager::createInputSystem( pl );

    mKeyboard = static_cast<OIS::Keyboard*>(mInputManager->createInputObject( OIS::OISKeyboard, true ));
    mMouse = static_cast<OIS::Mouse*>(mInputManager->createInputObject( OIS::OISMouse, true ));

    mMouse->setEventCallback(this);
    mKeyboard->setEventCallback(this);

    //Set initial mouse clipping size
    windowResized(mWindow);

    //Register as a Window listener
    Ogre::WindowEventUtilities::addWindowEventListener(mWindow, this);

    mTrayMgr = new OgreBites::SdkTrayManager("InterfaceName", mWindow, mMouse, this);
    mTrayMgr->showFrameStats(OgreBites::TL_BOTTOMLEFT);
    mTrayMgr->showLogo(OgreBites::TL_BOTTOMRIGHT);
    mTrayMgr->hideCursor();

    // create a params panel for displaying sample details
    Ogre::StringVector items;
    items.push_back("cam.pX");
    items.push_back("cam.pY");
    items.push_back("cam.pZ");
    items.push_back("");
    items.push_back("cam.oW");
    items.push_back("cam.oX");
    items.push_back("cam.oY");
    items.push_back("cam.oZ");
    items.push_back("");
    items.push_back("Filtering");
    items.push_back("Poly Mode");

    mDetailsPanel = mTrayMgr->createParamsPanel(OgreBites::TL_NONE, "DetailsPanel", 200, items);
    mDetailsPanel->setParamValue(9, "Bilinear");
    mDetailsPanel->setParamValue(10, "Solid");
    mDetailsPanel->hide();

    mRoot->addFrameListener(this);
}
//-------------------------------------------------------------------------------------
void CartoonHairSimulation::destroyScene(void)
{
}
//-------------------------------------------------------------------------------------
void CartoonHairSimulation::createViewports(void)
{
    // Create one viewport, entire window
    Ogre::Viewport* vp = mWindow->addViewport(mCamera);
    vp->setBackgroundColour(Ogre::ColourValue(0,0,0));

    // Alter the camera aspect ratio to match the viewport
    mCamera->setAspectRatio(
        Ogre::Real(vp->getActualWidth()) / Ogre::Real(vp->getActualHeight()));
}
//-------------------------------------------------------------------------------------
void CartoonHairSimulation::setupResources(void)
{
    // Load resource paths from config file
    Ogre::ConfigFile cf;
    cf.load(mResourcesCfg);

    // Go through all sections & settings in the file
    Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();

    Ogre::String secName, typeName, archName;
    while (seci.hasMoreElements())
    {
        secName = seci.peekNextKey();
        Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
        Ogre::ConfigFile::SettingsMultiMap::iterator i;
        for (i = settings->begin(); i != settings->end(); ++i)
        {
            typeName = i->first;
            archName = i->second;
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
                archName, typeName, secName);
        }
    }
}
//-------------------------------------------------------------------------------------
void CartoonHairSimulation::createResourceListener(void)
{

}
//-------------------------------------------------------------------------------------
void CartoonHairSimulation::loadResources(void)
{
    Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
}
//-------------------------------------------------------------------------------------
void CartoonHairSimulation::go(void)
{
#ifdef _DEBUG
    mResourcesCfg = "resources_d.cfg";
    mPluginsCfg = "plugins_d.cfg";
#else
    mResourcesCfg = "resources.cfg";
    mPluginsCfg = "plugins.cfg";
#endif

    if (!setup())
        return;

    mRoot->startRendering();

    // clean up
    destroyScene();
}
//-------------------------------------------------------------------------------------
bool CartoonHairSimulation::setup(void)
{
	//setup physics
	mCollisionConfig = new btSoftBodyRigidBodyCollisionConfiguration();
	mDispatcher = new btCollisionDispatcher(mCollisionConfig);
	mBroadphase = new btDbvtBroadphase();
	mConstraintSolver = new btSequentialImpulseConstraintSolver();
	mSoftBodySolver = new btDefaultSoftBodySolver();
	mWorld = new btSoftRigidDynamicsWorld(mDispatcher,mBroadphase,mConstraintSolver,mCollisionConfig,mSoftBodySolver);

    mRoot = new Ogre::Root(mPluginsCfg);

    setupResources();

    bool carryOn = configure();
    if (!carryOn) return false;

    chooseSceneManager();
    createCamera();
    createViewports();

    // Set default mipmap level (NB some APIs ignore this)
    Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);

    // Create any resource listeners (for loading screens)
    createResourceListener();
    // Load resources
    loadResources();

	// Create the scene
    createScene();

	createFrameListener();

    return true;
};
//-------------------------------------------------------------------------------------
void CartoonHairSimulation::createScene(void)
{
	if(mWindow->getNumViewports())
	{
		mWindow->getViewport(0)->setBackgroundColour(Ogre::ColourValue(0.39,0.58,0.92));
	}

	//model by http://www.turbosquid.com/FullPreview/Index.cfm/ID/403363
	Ogre::Entity* head = mSceneMgr->createEntity("Head", "oldheadbust.mesh");
	//head->setVisible(false);

	Ogre::SceneNode* headNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	headNode->attachObject(head);

	//setup debug drawer
	m_debugDrawer = new DebugDrawer(mSceneMgr);
	//m_debugDrawer->setDebugMode(btIDebugDraw::DBG_DrawWireframe);
	mWorld->setDebugDrawer(m_debugDrawer);

	headNode->attachObject(m_debugDrawer->getLinesManualObject());

	//based on http://www.ogre3d.org/tikiwiki/tiki-index.php?page=RetrieveVertexData
	size_t vertexCount, indexCount;
	Ogre::Vector3* vertices;
	unsigned long* indices;

	getMeshInformation(head->getMesh(),
		vertexCount,
		vertices,
		indexCount,
		indices,
		Ogre::Vector3::ZERO,
		Ogre::Quaternion::IDENTITY,
		Ogre::Vector3::UNIT_SCALE);

	/*Ogre::ManualObject *headPoints = new Ogre::ManualObject("headpoints");
	headPoints->begin("BaseWhiteNoLighting",Ogre::RenderOperation::OT_POINT_LIST);

	for(int i = 0 ; i < vertexCount ; i++)
	{
		headPoints->position(vertices[i]);
	}

	headPoints->end();

	headNode->attachObject(headPoints);*/

	//create collision rigid body - based upon https://bitbucket.org/alexeyknyshev/ogrebullet/src/555c70e80bf4/Collisions/src/Utils/OgreBulletCollisionsMeshToShapeConverter.cpp?at=master
	btConvexHullShape* complexHull = new btConvexHullShape((btScalar*) &vertices[0].x,vertexCount,sizeof(Ogre::Vector3));

	//btShapeHull* shapeHull = new btShapeHull(complexHull);
	//shapeHull->buildHull(complexHull->getMargin());
	//btConvexHullShape* headShape = new btConvexHullShape((btScalar*)shapeHull->getVertexPointer(),shapeHull->numVertices(),sizeof(Ogre::Vector3));

	//btSphereShape *sphereShape = new btSphereShape(0.26f);
	
	btDefaultMotionState *headMotionState = new btDefaultMotionState(
		btTransform(btQuaternion(0,0,0,1),btVector3(0,0,0)));
	btRigidBody::btRigidBodyConstructionInfo headConstructionInfo(0,headMotionState,complexHull,btVector3(0,0,0));

	btRigidBody* headRigidBody = new btRigidBody(headConstructionInfo);

	mWorld->addRigidBody(headRigidBody,BODY_GROUP, BODY_GROUP | HAIR_GROUP);

	//clean up
	/*delete complexHull;
	delete shapeHull;*/

	/*Ogre::ManualObject *headPoints = new Ogre::ManualObject("headpoints");
	headPoints->begin("BaseWhiteNoLighting",Ogre::RenderOperation::OT_POINT_LIST);

	for(int i = 0 ; i < headHull->getNumPoints() ; i++)
	{
		btVector3 * vert = headHull->getUnscaledPoints();
		headPoints->position(vert[i].x(),vert[i].y(),vert[i].z());
	}

	headPoints->end();

	headNode->attachObject(headPoints);*/

	//clean up
	delete[] vertices;
	delete[] indices;

	//setup head collision
	/*btCollisionShape* headShape = new btSphereShape(3.2);
	btDefaultMotionState* headMotionState = new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1),btVector3(0,2,0)));
	btRigidBody::btRigidBodyConstructionInfo headConstructionInfo(0,headMotionState,headShape,btVector3(0,0,0));
	btRigidBody* headRigidBody = new btRigidBody(headConstructionInfo);

	mWorld->addRigidBody(headRigidBody);*/


	//setup dynamic hair
	m_hairModel = new HairModel("../hair/hairtest2.xml",mSceneMgr,mWorld);
	headNode->attachObject(m_hairModel->getManualObject());

	//if reduce to the correct size in the simulation - the collision becomes inaccurate - instead scaling the simulation
	//http://www.bulletphysics.org/mediawiki-1.5.8/index.php?title=Scaling_The_World

	mWorld->setGravity(mWorld->getGravity()*m_hairModel->getSimulationScale());

	// Set ambient light
	mSceneMgr->setAmbientLight(Ogre::ColourValue(0.5, 0.5, 0.5));

	// Create a light
	Ogre::Light* l = mSceneMgr->createLight("MainLight");
	l->setPosition(20,80,50);

	////load hair
	//tinyxml2::XMLDocument doc;
	//doc.LoadFile("../hair/hairtest.xml");

	////get the hair node
	//tinyxml2::XMLElement *hair = doc.FirstChildElement();
	//OutputDebugString(hair->Name());

	////get the first strand
	//tinyxml2::XMLElement *strand = hair->FirstChildElement();
	//OutputDebugString(strand->Name());

	////create manual hair object
	//hairMesh = mSceneMgr->createManualObject("hair");
	//hairMesh->setDynamic(true);

	////iterate through the strands
	//for(strand ; strand ; strand = strand->NextSiblingElement())
	//{
	//	
	//	hairMesh->begin("BaseWhiteNoLighting",Ogre::RenderOperation::OT_LINE_STRIP);
	//	//iterate through hair particles - first one is the root and should be fixed in position
	//	tinyxml2::XMLElement *particle = strand->FirstChildElement();
	//	for(particle ; particle ; particle = particle->NextSiblingElement())
	//	{
	//		hairMesh->colour(1.0,0,0);
	//		hairMesh->position(
	//			particle->FloatAttribute("x"),
	//			particle->FloatAttribute("y"),
	//			particle->FloatAttribute("z")
	//		);
	//	}
	//	hairMesh->end();
	//}


	//headNode->attachObject(hairMesh);

	//http://www.youtube.com/watch?v=d7_lJJ_j2NE
	/*float s=4, h=20;
	body = btSoftBodyHelpers::CreatePatch(mWorld->getWorldInfo(),
		btVector3(-s,h,-s),btVector3(s,h,-s),btVector3(-s,h,s),btVector3(s,h,s),
		50,50,4+8,true);
	body->m_cfg.viterations = 10;
	body->m_cfg.piterations = 10;
	mWorld->addSoftBody(body);

	plane = mSceneMgr->createManualObject("plane");
	plane->setDynamic(true);
	plane->begin("BaseWhiteNoLighting",Ogre::RenderOperation::OT_TRIANGLE_LIST);
	for(int i = 0 ; i < body->m_faces.size() ; i++)
	{
		for(int j = 0 ; j < 3 ; j++)
		{
			plane->position(body->m_faces[i].m_n[j]->m_x.x(),
				body->m_faces[i].m_n[j]->m_x.y(),
				body->m_faces[i].m_n[j]->m_x.z());
		}
	}
	plane->end();
	mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(plane);*/
}
//-------------------------------------------------------------------------------------
bool CartoonHairSimulation::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
    if(mWindow->isClosed())
        return false;

    if(mShutDown)
        return false;

    //Need to capture/update each device
    mKeyboard->capture();
    mMouse->capture();

    mTrayMgr->frameRenderingQueued(evt);

    if (!mTrayMgr->isDialogVisible())
    {
        mCameraMan->frameRenderingQueued(evt);   // if dialog isn't up, then update the camera
        if (mDetailsPanel->isVisible())   // if details panel is visible, then update its contents
        {
            mDetailsPanel->setParamValue(0, Ogre::StringConverter::toString(mCamera->getDerivedPosition().x));
            mDetailsPanel->setParamValue(1, Ogre::StringConverter::toString(mCamera->getDerivedPosition().y));
            mDetailsPanel->setParamValue(2, Ogre::StringConverter::toString(mCamera->getDerivedPosition().z));
            mDetailsPanel->setParamValue(4, Ogre::StringConverter::toString(mCamera->getDerivedOrientation().w));
            mDetailsPanel->setParamValue(5, Ogre::StringConverter::toString(mCamera->getDerivedOrientation().x));
            mDetailsPanel->setParamValue(6, Ogre::StringConverter::toString(mCamera->getDerivedOrientation().y));
            mDetailsPanel->setParamValue(7, Ogre::StringConverter::toString(mCamera->getDerivedOrientation().z));
        }
    }

	
	float timestep = evt.timeSinceLastFrame;
	//physics update - note:  must (timeStep < maxSubSteps*fixedTimeStep) == true according to http://bulletphysics.org/mediawiki-1.5.8/index.php/Stepping_The_World
	//if odd physics problems arise - consider adding paramters for maxSubstep and fixedTimeStep

	mWorld->stepSimulation(timestep);
	m_hairModel->updateManualObject();

	m_debugDrawer->begin();
	mWorld->debugDrawWorld();
	m_debugDrawer->end();

	/*plane->beginUpdate(0);
	for(int i = 0 ; i < body->m_faces.size() ; i++)
	{
		for(int j = 0 ; j < 3 ; j++)
		{
			plane->position(body->m_faces[i].m_n[j]->m_x.x(),
				body->m_faces[i].m_n[j]->m_x.y(),
				body->m_faces[i].m_n[j]->m_x.z());
		}
	}
	plane->end();*/

	//TO DO: add any methods you would like to be called

    return true;
}
//-------------------------------------------------------------------------------------
bool CartoonHairSimulation::keyPressed( const OIS::KeyEvent &arg )
{
    if (mTrayMgr->isDialogVisible()) return true;   // don't process any more keys if dialog is up

    if (arg.key == OIS::KC_F)   // toggle visibility of advanced frame stats
    {
        mTrayMgr->toggleAdvancedFrameStats();
    }
    else if (arg.key == OIS::KC_G)   // toggle visibility of even rarer debugging details
    {
        if (mDetailsPanel->getTrayLocation() == OgreBites::TL_NONE)
        {
            mTrayMgr->moveWidgetToTray(mDetailsPanel, OgreBites::TL_TOPRIGHT, 0);
            mDetailsPanel->show();
        }
        else
        {
            mTrayMgr->removeWidgetFromTray(mDetailsPanel);
            mDetailsPanel->hide();
        }
    }
    else if (arg.key == OIS::KC_T)   // cycle polygon rendering mode
    {
        Ogre::String newVal;
        Ogre::TextureFilterOptions tfo;
        unsigned int aniso;

        switch (mDetailsPanel->getParamValue(9).asUTF8()[0])
        {
        case 'B':
            newVal = "Trilinear";
            tfo = Ogre::TFO_TRILINEAR;
            aniso = 1;
            break;
        case 'T':
            newVal = "Anisotropic";
            tfo = Ogre::TFO_ANISOTROPIC;
            aniso = 8;
            break;
        case 'A':
            newVal = "None";
            tfo = Ogre::TFO_NONE;
            aniso = 1;
            break;
        default:
            newVal = "Bilinear";
            tfo = Ogre::TFO_BILINEAR;
            aniso = 1;
        }

        Ogre::MaterialManager::getSingleton().setDefaultTextureFiltering(tfo);
        Ogre::MaterialManager::getSingleton().setDefaultAnisotropy(aniso);
        mDetailsPanel->setParamValue(9, newVal);
    }
    else if (arg.key == OIS::KC_R)   // cycle polygon rendering mode
    {
        Ogre::String newVal;
        Ogre::PolygonMode pm;

        switch (mCamera->getPolygonMode())
        {
        case Ogre::PM_SOLID:
            newVal = "Wireframe";
            pm = Ogre::PM_WIREFRAME;
            break;
        case Ogre::PM_WIREFRAME:
            newVal = "Points";
            pm = Ogre::PM_POINTS;
            break;
        default:
            newVal = "Solid";
            pm = Ogre::PM_SOLID;
        }

        mCamera->setPolygonMode(pm);
        mDetailsPanel->setParamValue(10, newVal);
    }
    else if(arg.key == OIS::KC_F5)   // refresh all textures
    {
        Ogre::TextureManager::getSingleton().reloadAll();
    }
    else if (arg.key == OIS::KC_SYSRQ)   // take a screenshot
    {
        mWindow->writeContentsToTimestampedFile("screenshot", ".jpg");
    }
    else if (arg.key == OIS::KC_ESCAPE)
    {
        mShutDown = true;
    }

    mCameraMan->injectKeyDown(arg);
    return true;
}

bool CartoonHairSimulation::keyReleased( const OIS::KeyEvent &arg )
{
    mCameraMan->injectKeyUp(arg);
    return true;
}

bool CartoonHairSimulation::mouseMoved( const OIS::MouseEvent &arg )
{
    if (mTrayMgr->injectMouseMove(arg)) return true;
    mCameraMan->injectMouseMove(arg);
    return true;
}

bool CartoonHairSimulation::mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
    if (mTrayMgr->injectMouseDown(arg, id)) return true;
    mCameraMan->injectMouseDown(arg, id);
    return true;
}

bool CartoonHairSimulation::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
    if (mTrayMgr->injectMouseUp(arg, id)) return true;
    mCameraMan->injectMouseUp(arg, id);
    return true;
}

//Adjust mouse clipping area
void CartoonHairSimulation::windowResized(Ogre::RenderWindow* rw)
{
    unsigned int width, height, depth;
    int left, top;
    rw->getMetrics(width, height, depth, left, top);

    const OIS::MouseState &ms = mMouse->getMouseState();
    ms.width = width;
    ms.height = height;
}

//Unattach OIS before window shutdown (very important under Linux)
void CartoonHairSimulation::windowClosed(Ogre::RenderWindow* rw)
{
    //Only close for window that created OIS (the main window in these demos)
    if( rw == mWindow )
    {
        if( mInputManager )
        {
            mInputManager->destroyInputObject( mMouse );
            mInputManager->destroyInputObject( mKeyboard );

            OIS::InputManager::destroyInputSystem(mInputManager);
            mInputManager = 0;
        }
    }
}


#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
    INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
    int main(int argc, char *argv[])
#endif
    {
        // Create application object
        CartoonHairSimulation app;

        try {
            app.go();
        } catch( Ogre::Exception& e ) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
            MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
            std::cerr << "An exception has occured: " <<
                e.getFullDescription().c_str() << std::endl;
#endif
        }

        return 0;
    }

#ifdef __cplusplus
}
#endif
