#include "world.h"
#include <glm/glm.hpp>
#include <math.h>
#include <bits/stdc++.h>
#include "material.h"


Color Material::shade(const Ray& incident, const bool isSolid) const
{
	// Initialize ambient (a), specular (s), and diffuse (d) colors.
	Color a(0), s(0), d(0);

	// Calculate the normal, view direction, light position, and light direction.
	LightSource* lsr = world -> lightSourceList[0];
	Vector3D nor = incident.getNormal(), v = incident.getDirection()*(-1),lightps = lsr -> getPosition();
	Vector3D ldr = unitVector(lightps - incident.getPosition());

	// Get the intensity of the light source.
	Color ints = lsr -> getIntensity();

	// Calculate the half vector between light and view direction.
	Vector3D half_vec = unitVector(ldr + v);


	// Calculating ambient, specular and diffuse
	a = ka*color*ints;
	s = ints*ks*color*pow(glm::max((double)0,dotProduct(nor,half_vec)),n);
	
	d = ints*kd*color*(glm::max((double)0,dotProduct(nor,ldr)));

	Ray shadow(incident.getPosition() + 0.1*ldr,ldr);
	world -> firstIntersection(shadow);

	if(shadow.didHit()){
		return (a + s);
	}

	// Return the sum of ambient, specular, and diffuse components.
	return (a + s + d );

}
