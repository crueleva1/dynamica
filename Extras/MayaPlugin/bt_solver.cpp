/*
Bullet Continuous Collision Detection and Physics Library Maya Plugin
Copyright (c) 2008 Walt Disney Studios
 
This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising
from the use of this software.
Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it freely,
subject to the following restrictions:
 
1. The origin of this software must not be misrepresented; you must
not claim that you wrote the original software. If you use this
software in a product, an acknowledgment in the product documentation
would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must
not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
 
Written by: Nicola Candussi <nicola@fluidinteractive.com>

Modified by Roman Ponomarev <rponom@gmail.com>
01/22/2010 : Constraints reworked
01/27/2010 : Replaced COLLADA export with Bullet binary export
02/18/2010 : Re-enabled COLLADA export as option (next to Bullet binary export)
*/

//bt_solver.cpp

#include "LinearMath/btSerializer.h"
#include "bt_solver.h"
#include "../BulletColladaConverter/ColladaConverter.h"
#include <string.h>
#include <stdio.h>


void bt_solver_t::export_collada_file(const char* fileName)
{
#ifdef BT_USE_COLLADA
	ColladaConverter tmpConverter(m_dynamicsWorld.get());
	tmpConverter.save(fileName);
#endif
}

void bt_solver_t::import_collada_file(const char* filename)
{
	//	ColladaConverter tmpConverter(m_dynamicsWorld.get());
	//	tmpConverter.save(fileName);
}

class MySerializer : public btDefaultSerializer
{
	bt_solver_t* m_solver;

public:
	MySerializer(bt_solver_t* solver)
		:btDefaultSerializer(),
		m_solver(solver)
	{
	}

	virtual	const char*	findNameForPointer(const void* ptr) const
	{
		const char*const * namePtr = m_solver->m_nameMap.find(ptr);
		if (namePtr && *namePtr)
			return *namePtr;
		return 0;

	}

	
};

void bt_solver_t::export_bullet_file(const char* fileName)
{
	FILE* f2 = fopen(fileName,"wb");
	if(f2 == NULL)
	{
	    fprintf(stderr,"Error: Can't open file %s for writing\n", fileName);
		return;
	}

	btDefaultSerializer* serializer = new MySerializer(this);

	m_dynamicsWorld->serialize(serializer);
	fwrite(serializer->getBufferPointer(),serializer->getCurrentBufferSize(),1,f2);
	fclose(f2);
	delete serializer;
}



void bt_solver_t::register_name(const void* pointer,const char* objectName)
{
	if (objectName)
	{
		int nameLen = strlen(objectName);
		if (nameLen>0)
		{
			char* newName = (char*)malloc(nameLen+1);
			memcpy(newName ,objectName,nameLen);
			newName[nameLen]=0;
			m_nameMap.insert(pointer,newName);
		}
	}
}

void bt_solver_t::import_bullet_file(const char* filename)
{
//todo: need to create actual bodies etc
//	ColladaConverter tmpConverter(m_dynamicsWorld.get());
//	tmpConverter.load(filename);
}


class bt_debug_draw : public btIDebugDraw
{
	int m_debugMode;
public:
	virtual void	drawLine(const btVector3& from,const btVector3& to,const btVector3& color)
	{
		glBegin(GL_LINES);
			glColor3f(color.getX(), color.getY(), color.getZ());
			glVertex3d(from.getX(), from.getY(), from.getZ());
			glVertex3d(to.getX(), to.getY(), to.getZ());
		glEnd();
	}
	virtual void	drawContactPoint(const btVector3& PointOnB,const btVector3& normalOnB,btScalar distance,int lifeTime,const btVector3& color)
	{
		return;
	}
	virtual void	reportErrorWarning(const char* warningString)
	{
		return;
	}
	virtual void	draw3dText(const btVector3& location,const char* textString) 
	{
		return;
	}
	virtual void	setDebugMode(int debugMode) { m_debugMode = debugMode; }
	virtual int		getDebugMode() const  { return m_debugMode; }
};



bt_solver_t::bt_solver_t():
	m_broadphase(new btDbvtBroadphase()),  
          m_solver(new btSequentialImpulseConstraintSolver),
            m_collisionConfiguration(new btDefaultCollisionConfiguration),
            m_dispatcher(new btCollisionDispatcher(m_collisionConfiguration.get())),
            m_dynamicsWorld(new btDiscreteDynamicsWorld(m_dispatcher.get(),
                                                        m_broadphase.get(),
                                                        m_solver.get(),
                                                        m_collisionConfiguration.get()))
{

    //register algorithm for concave meshes
    btGImpactCollisionAlgorithm::registerAlgorithm(m_dispatcher.get());

    m_dynamicsWorld->setGravity(btVector3(0, -9.81f, 0));

  //  m_dynamicsWorld->getSolverInfo().m_splitImpulse = true;

	bt_debug_draw* dbgDraw = new bt_debug_draw();
	m_dynamicsWorld->setDebugDrawer(dbgDraw);

	//for now, allocated 15MB for serialization structures. todo: compute this memory
	
}


void bt_solver_t::debug_draw(int dbgMode)
{
	m_dynamicsWorld->getDebugDrawer()->setDebugMode(dbgMode);
	m_dynamicsWorld->debugDrawWorld();
}

