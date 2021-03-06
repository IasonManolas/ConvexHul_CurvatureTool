﻿#define  NOMINMAX // Fixes a problem on windows
#include "GeoLab_Apallaktiki_03_XXXX.h"
#include "utils.h"
#include "canvas.h"
#include <iostream>
#include <fstream>
#include <iostream>
#include <cstring>
#include <string>
#include <MathGeoLib/MathGeoLib.h>
#include <algorithm>
#include <vector>
#include <cmath>
#include "Curv_Of_3D.h"
#include "Curv_Of_2D.h"
#include "Convex_Hull.h"
#include "Curv_Of_Convex_Hull.h"
#include "Retriangulation_Of_CH.h"
#include "Curv_Of_ReTriCH.h"
#include "Topika_Megista_MeanCurv.h"
#include "k_Daktylios.h"
#define CONFIG_FILE_PATH "../Apallaktiki_03/config.txt"
#define POLYGON_FILENAME  "../Apallaktiki_03/polygons/polygon.txt"
#define MIN_POINT_DIST_PIXELS 5

#define FLAG_SHOW_AXES       1
#define FLAG_SHOW_AABB       2
#define FLAG_SHOW_WIRE       4
#define FLAG_SHOW_SOLID      8
#define FLAG_SHOW_NORMALS   16
#define FLAG_SHOW_2D_POL	32
#define FLAG_SHOW_2D_CURV	64
#define FLAG_SHOW_MODEL		128
#define FLAG_SHOW_3D_CURV_MEAN 256
#define FLAG_SHOW_3D_CURV_GAUSSIAN 512
#define FLAG_SHOW_3D_CONVEX_HULL 512*2
#define FLAG_SHOW_CH_CURV_MEAN 512*4
#define FLAG_SHOW_CH_CURV_GAUSSIAN 512*8
#define FLAG_SHOW_CURV_OF_RETRI_CH 512*16
#define FLAG_SHOW_EXTREMA_POINTS 512*32
using namespace math;
using namespace vvr;
int daktylios;

Simple3DScene::Simple3DScene()
{
    // Load settings.
    m_settings          = Settings(getExePath() + CONFIG_FILE_PATH);
    m_bg_col            = Colour(m_settings.getStr("color_bg"));
    m_obj_col           = Colour(m_settings.getStr("color_obj"));
    m_perspective_proj  = m_settings.getBool("perspective_proj");
    m_style_flag        = FLAG_SHOW_SOLID;

    // Scene rotation.
    const double def_rot_x = m_settings.getDbl("def_rot_x");
    const double def_rot_y = m_settings.getDbl("def_rot_y");
    const double def_rot_z = m_settings.getDbl("def_rot_z");
    m_globRot_def = Vec3d(def_rot_x, def_rot_y, def_rot_z);
    m_globRot = m_globRot_def;

    // Load 3D models.
    const string objDir = getExePath() + m_settings.getStr("obj_dir");
    const string objFile = getExePath() +  m_settings.getStr("obj_file");
    
    // H teleytaia parametros kathorizei an ta trigwna dinontai se CW/CCW fora.
    // An to 3D antikeimenou emfanizetai xwris skies, allakse ayti tin parametro.
    m_model = Mesh(objDir, objFile, "", false); 

    // Fortwse to polygwno apo tin mnimi
    loadPolygonFromFile(getExePath() + POLYGON_FILENAME);
    b_show_pts = true;
}
void Simple3DScene::resize()
{
    // Making FIRST PASS static and initialing it to true we make
    // sure that the if block will be executed only once.
    static bool FLAG_FIRST_PASS = true;

    if (FLAG_FIRST_PASS)
    {
        m_sphere_rad = getSceneWidth()/8;
        m_model.setBigSize(getSceneWidth()/6);
        m_model.centerAlign();

        vector<Vec3d> &vertices = m_model.getVertices();

        for (unsigned vi=0 ; vi<vertices.size() ; vi++)
        {
            Vec3d &vertex = vertices[vi];

            // Here you can process the vertices of the icosahedron model
            //...
            //...
            //...

        }

        m_model.update();

    }

    FLAG_FIRST_PASS = false;
}
void Simple3DScene::draw()
{
	static bool FLAG_FIRST_PASS = true;
	if (FLAG_FIRST_PASS)
	{	
		CurvOf3DModel();
		ConvexHull3D();
		curvOfConvexHull();
		Retriangulation_Of_CH();
		Topika_Megista();
		Curv_Of_Retri_CH();
	}
	FLAG_FIRST_PASS = false;
	if (m_style_flag & FLAG_SHOW_MODEL) //model drawing 
	{
		if (m_style_flag & FLAG_SHOW_SOLID)     m_model.draw(m_obj_col, SOLID);
		if (m_style_flag & FLAG_SHOW_WIRE)      m_model.draw(Colour::black, WIRE);
		if (m_style_flag & FLAG_SHOW_NORMALS)   m_model.draw(Colour::black, NORMALS);
		if (m_style_flag & FLAG_SHOW_AXES)      m_model.draw(Colour::black, AXES);
		if (m_style_flag & FLAG_SHOW_AABB)      m_model.draw(Colour::black, BOUND);
		if (m_style_flag &FLAG_SHOW_3D_CURV_GAUSSIAN) DrawKg();
		if (m_style_flag &FLAG_SHOW_3D_CURV_MEAN) DrawKm();
		

	}
	if (m_style_flag &FLAG_SHOW_3D_CONVEX_HULL) DrawCH();
	if (m_style_flag &FLAG_SHOW_CH_CURV_MEAN) DrawKmCH();

	if (m_style_flag &FLAG_SHOW_2D_CURV) if(m_pts.size()>0) Curvature2D(); // O(n)
	if (m_style_flag &FLAG_SHOW_CURV_OF_RETRI_CH) DrawRetriKmCH();
	if (m_style_flag &FLAG_SHOW_EXTREMA_POINTS) DrawExtremaPoints(daktylios);


}
void Simple3DScene::keyEvent(unsigned char key, bool up, int modif)
{
    Scene::keyEvent(key, up, modif);
    key = tolower(key);

	switch (key)
	{
	case 'a': m_style_flag ^= FLAG_SHOW_AXES; break;
	case 'w': m_style_flag ^= FLAG_SHOW_WIRE; break;
	case 's': m_style_flag ^= FLAG_SHOW_SOLID; break;
	case 'n': m_style_flag ^= FLAG_SHOW_NORMALS; break;
	case 'b': m_style_flag ^= FLAG_SHOW_AABB; break;
	case 'p': b_show_pts ^= true; break;
	case 'f': savePolygonToFile(); break;
	case 'r': m_pts.clear(); break;
	case 'm': m_style_flag ^= FLAG_SHOW_MODEL; break;


	case '1': m_style_flag ^= FLAG_SHOW_2D_CURV; break;
	case '2': 
		if (ctrlDown(modif)) { m_style_flag ^= FLAG_SHOW_3D_CURV_MEAN; break; }
		else { m_style_flag ^= FLAG_SHOW_3D_CURV_GAUSSIAN; break; }
	case '3':
		m_style_flag ^= FLAG_SHOW_3D_CONVEX_HULL;
		if (ctrlDown(modif)) m_style_flag ^= FLAG_SHOW_CH_CURV_MEAN;
		break;
	case'4':
		if (ctrlDown(modif)){ReRetriangulate();Curv_Of_Retri_CH();} 
		else{ m_style_flag ^= FLAG_SHOW_CURV_OF_RETRI_CH; m_style_flag &= ~FLAG_SHOW_3D_CONVEX_HULL; m_style_flag &= ~FLAG_SHOW_CH_CURV_MEAN; }
		break;
	case'5': m_style_flag ^= FLAG_SHOW_EXTREMA_POINTS; daktylios = megethos_Daktyliou();
			break;
	case'6':
		k_Daktylios(megethos_Daktyliou(), katwfli_Kampylothtas()); 
		CurvOf3DModel();
		break;
	}

}
void Simple3DScene::mousePressed(int x, int y, int modif)
{
    if (ctrlDown(modif)) {
        Scene::mousePressed(x, y, modif);
        return;
    }

    if (altDown(modif)) {
        float xf = x; float yf = y;
        m_pts.push_back(C2DPoint(xf, yf));
    }

}
void Simple3DScene::savePolygonToFile()
{
	string filename = POLYGON_FILENAME;
	filename = getExePath() + filename;
	std::cout << "Saving to " << filename << std::endl;

	FILE* file = fopen(filename.c_str(), "w");
	if (!file) throw "Cannot open <" + filename + "> for writing";

	for (int pi = 0; pi < m_pts.size(); pi++) {
		C2DPoint &p = m_pts[pi];
		fprintf(file, "%f %f \n", p.x, p.y);
	}

	fclose(file);
}

void Simple3DScene::mouseMoved(int x, int y, int modif)
{
    if (!altDown(modif)) {
        Scene::mouseMoved(x, y, modif);
        return;
    }
    
    float xf = x; float yf = y;

    float d, dmin;
    dmin = MIN_POINT_DIST_PIXELS;

    if (!m_pts.empty() && !m_pts.empty()) {
        double lx = m_pts.back().x;
        double ly = m_pts.back().y;
        d = sqrt((double)(SQUARE(lx - xf) + SQUARE(ly - yf)));
    }
    else {
        d = 10000;
    }

    if (d > dmin) {
        m_pts.push_back(C2DPoint(xf, yf));
    }
}
void Simple3DScene::loadPolygonFromFile(string filename)
{
    FILE* file = fopen(filename.c_str(), "r");
    if (!file) throw "Cannot open <" + filename + "> for reading";

    m_pts.clear();

    char line[1024];
    while (fgets(line, 1023, file)) {
        int len;
        if ((len = strlen(line))<1) continue;
        if (line[len - 1] == '\n') line[len - 1] = 0;
        float x, y;
        sscanf(line, "%f %f", &x, &y);
        m_pts.push_back(C2DPoint(x, y));
    }

}
void Simple3DScene::pixelCoordsToSceneCoords(float &x, float &y)
{
    x = getSceneWidth() / getViewportWidth()  *  x;
    y = getSceneHeight() / getViewportHeight() *  y;
}

int main(int argc, char* argv[])
{
    try {
        return vvr::mainLoop(argc, argv, new Simple3DScene);
    }
    catch (std::string exc) {
        std::cerr << exc << std::endl;
        return 1;
    }
}
