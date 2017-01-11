#include "clothdesigner.h"
#include <QGridLayout>
#include "global_data_holder.h"
#include <exception>
#include "viewer2d.h"
#include "viewer3d.h"
#include "cloth\HistoryStack.h"
#include "cloth\clothPiece.h"
#include "cloth\clothManager.h"
#include "cloth\TransformInfo.h"
#include "cloth\graph\Graph.h"
#include "Renderable\ObjMesh.h"
#include "cloth\SmplManager.h"

ClothDesigner::ClothDesigner(QWidget *parent)
: QMainWindow(parent), m_projectSaved(false)
{
	ui.setupUi(this);
	setAcceptDrops(true);
	ui.centralWidget->setLayout(new QGridLayout());
	m_splitter = new QSplitter(ui.centralWidget);
	ui.centralWidget->layout()->addWidget(m_splitter);
	m_widget2d = new Viewer2d(m_splitter);
	m_widget3d = new Viewer3d(m_splitter);
	m_splitter->addWidget(m_widget3d);
	m_splitter->addWidget(m_widget2d);

	init3dActions();
	init2dActions();

	g_dataholder.init();
	g_dataholder.m_historyStack->init(g_dataholder.m_clothManager.get(), m_widget2d);
	m_widget3d->init(g_dataholder.m_clothManager.get(), this);
	m_widget2d->init(g_dataholder.m_clothManager.get(), this);

	setupSmplUI();
	updateUiByParam();
	m_simulateTimer = startTimer(1);
	m_fpsTimer = startTimer(200);
}

ClothDesigner::~ClothDesigner()
{

}

void ClothDesigner::timerEvent(QTimerEvent* ev)
{
	if (ev->timerId() == m_simulateTimer)
	{
		if (g_dataholder.m_clothManager->getSimulationMode() == ldp::SimulationOn)
		{
			try
			{
				g_dataholder.m_clothManager->simulationUpdate();
				m_widget3d->updateGL();
			} catch (std::exception e)
			{
				std::cout << e.what() << std::endl;
			} catch (...)
			{
				std::cout << "timerEvent: unknown error" << std::endl;
			}
		}
	}

	if (ev->timerId() == m_fpsTimer)
		setWindowTitle(QString().sprintf("fps: %f", g_dataholder.m_clothManager->getFps()));
}

void ClothDesigner::closeEvent(QCloseEvent* ev)
{
	g_dataholder.saveLastDirs();
}

void ClothDesigner::dragEnterEvent(QDragEnterEvent* event)
{
	if (event->mimeData()->hasUrls())
	{
		QList<QUrl> urls = event->mimeData()->urls();
		if (urls[0].fileName().toLower().endsWith(".svg")
			|| urls[0].fileName().endsWith(".xml"))
			event->acceptProposedAction();
	}
}

void ClothDesigner::dropEvent(QDropEvent* event)
{
	QUrl url = event->mimeData()->urls()[0];
	QString name = url.toLocalFile();
	try
	{
		if (name.toLower().endsWith(".svg"))
			loadSvg(name);
		else if (name.toLower().endsWith(".xml"))
			loadProjectXml(name);
	} catch (std::exception e)
	{
		std::cout << e.what() << std::endl;
	} catch (...)
	{
		std::cout << "unknown error" << std::endl;
	}

	event->acceptProposedAction();
}

void ClothDesigner::loadSvg(QString name)
{
	g_dataholder.loadSvg(name.toStdString());
	g_dataholder.m_lastSvgDir = name.toStdString();
	g_dataholder.saveLastDirs();
	g_dataholder.m_historyStack->push("init", ldp::HistoryStack::TypeGeneral);
	m_widget3d->init(g_dataholder.m_clothManager.get(), this);
	m_widget2d->init(g_dataholder.m_clothManager.get(), this);
	m_widget2d->setEventHandleType(Abstract2dEventHandle::ProcessorTypeEditPattern);
	updateUiByParam();
	m_widget2d->updateGL();
	m_widget3d->updateGL();
}

void ClothDesigner::loadProjectXml(QString name)
{
	g_dataholder.m_clothManager->fromXml(name.toStdString());
	g_dataholder.m_lastProXmlDir = name.toStdString();
	g_dataholder.saveLastDirs();
	g_dataholder.m_historyStack->push("load project", ldp::HistoryStack::TypeGeneral);
	g_dataholder.m_clothManager->simulationInit();
	m_widget3d->init(g_dataholder.m_clothManager.get(), this);
	m_widget2d->init(g_dataholder.m_clothManager.get(), this);
	m_widget2d->setEventHandleType(Abstract2dEventHandle::ProcessorTypeEditPattern);
	updateUiByParam();
	m_widget2d->updateGL();
	m_widget3d->updateGL();
}

//////main menu/////////////////////////////////////////////////////////////////////////////////
void ClothDesigner::on_actionLoad_svg_triggered()
{
	try
	{
		QString name = QFileDialog::getOpenFileName(this, "load svg", g_dataholder.m_lastSvgDir.c_str(), "*.svg");
		if (name.isEmpty())
			return;
		loadSvg(name);
		m_projectSaved = false;
	} catch (std::exception e)
	{
		std::cout << e.what() << std::endl;
	} catch (...)
	{
		std::cout << "unknown error" << std::endl;
	}
}

void ClothDesigner::on_actionLoad_project_triggered()
{
	try
	{
		QString name = QFileDialog::getOpenFileName(this, "Load Project", g_dataholder.m_lastProXmlDir.c_str(), "*.xml");
		if (name.isEmpty())
			return;
		loadProjectXml(name);
		m_projectSaved = true;
	} catch (std::exception e)
	{
		std::cout << e.what() << std::endl;
	} catch (...)
	{
		std::cout << "unknown error" << std::endl;
	}
}

void ClothDesigner::saveProject(const std::string& fileName)
{
	g_dataholder.saveLastDirs();
	g_dataholder.m_clothManager->toXml(fileName);
	std::cout << "Save project file:" << fileName << std::endl;
}

void ClothDesigner::saveProjectAs()
{
	QString name = QFileDialog::getSaveFileName(this, "Save Project", g_dataholder.m_lastProXmlDir.c_str(), "*.xml");
	if (name.isEmpty())
		return;
	if (!name.toLower().endsWith(".xml"))
		name.append(".xml");
	g_dataholder.m_lastProXmlDir = name.toStdString();
	saveProject(name.toStdString());
}

void ClothDesigner::on_actionSave_project_triggered()
{
	try
	{
		if (!m_projectSaved)
			saveProjectAs();
		else
			saveProject(g_dataholder.m_lastProXmlDir);
		m_projectSaved = true;
	} catch (std::exception e)
	{
		std::cout << e.what() << std::endl;
	} catch (...)
	{
		std::cout << "unknown error" << std::endl;
	}
}

void ClothDesigner::on_actionExport_training_data_triggered()
{
	//load project xml
	try
	{
		QString name = QFileDialog::getOpenFileName(this, "Load Project", g_dataholder.m_lastProXmlDir.c_str(), "*.xml");
		if (name.isEmpty())
			return;
		loadProjectXml(name);
		m_projectSaved = true;
	}
	catch (std::exception e)
	{
		std::cout << e.what() << std::endl;
	}
	catch (...)
	{
		std::cout << "unknown error" << std::endl;
	}




}

void  ClothDesigner::on_actionSave_as_triggered()
{
	try
	{
		saveProjectAs();
		m_projectSaved = true;
	}
	catch (std::exception e)
	{
		std::cout << e.what() << std::endl;
	}
	catch (...)
	{
		std::cout << "unknown error" << std::endl;
	}
}

void ClothDesigner::on_actionExport_cloth_mesh_triggered()
{
	try
	{
		QString name = QFileDialog::getSaveFileName(this, "Export body mesh", g_dataholder.m_lastProXmlDir.c_str(), "*.obj");
		if (name.isEmpty())
			return;
		if (!name.toLower().endsWith(".obj"))
			name.append(".obj");

		g_dataholder.m_lastProXmlDir = name.toStdString();
		g_dataholder.saveLastDirs();

		ObjMesh mesh;
		g_dataholder.m_clothManager->exportClothsMerged(mesh, true);
		mesh.saveObj(name.toStdString().c_str());

		//ObjMesh mesh2d;
		//mesh2d.vertex_list.resize(mesh.vertex_list.size());
		//mesh2d.face_list = mesh.face_list;
		//for (size_t i = 0; i < mesh.vertex_list.size(); i++)
		//	mesh2d.vertex_list[i] = Float3(mesh.vertex_texture_list[i][0], mesh.vertex_texture_list[i][1], 0);
		//mesh2d.saveObj(QDir::cleanPath(QFileInfo(name).absolutePath()+QDir::separator()
		//	+ QFileInfo(name).baseName() + "_2d.obj").toStdString().c_str());
	} catch (std::exception e)
	{
		std::cout << e.what() << std::endl;
	} catch (...)
	{
		std::cout << "unknown error" << std::endl;
	}
}

void ClothDesigner::on_actionExport_body_mesh_triggered()
{
	try
	{
		QString name = QFileDialog::getSaveFileName(this, "Export body mesh", g_dataholder.m_lastProXmlDir.c_str(), "*.obj");
		if (name.isEmpty())
			return;
		if (!name.toLower().endsWith(".obj"))
			name.append(".obj");

		g_dataholder.m_lastProXmlDir = name.toStdString();
		g_dataholder.saveLastDirs();

		g_dataholder.m_clothManager->bodyMesh()->saveObj(name.toStdString().c_str());
	} catch (std::exception e)
	{
		std::cout << e.what() << std::endl;
	} catch (...)
	{
		std::cout << "unknown error" << std::endl;
	}
}

void ClothDesigner::on_actionPlace_3d_by_2d_triggered()
{
	try
	{
		g_dataholder.m_clothManager->resetCloths3dMeshBy2d();
		m_widget3d->updateGL();
	} catch (std::exception e)
	{
		std::cout << e.what() << std::endl;
	} catch (...)
	{
		std::cout << "unknown error" << std::endl;
	}
}

void ClothDesigner::pushHistory(QString name, ldp::HistoryStack::Type type)
{
	try
	{
		g_dataholder.m_historyStack->push(name.toStdString(), type);
	} 
	catch (std::exception e)
	{
		std::cout << e.what() << std::endl;
	}
	catch (...)
	{
		std::cout << "pushHistory(): unknown exception!" << std::endl;
	}
}

void ClothDesigner::on_actionPrev_triggered()
{
	try
	{
		g_dataholder.m_historyStack->stepBackward();
		m_widget2d->updateGL();
		m_widget3d->updateGL();
		m_widget3d->getEventHandle(m_widget3d->getEventHandleType())->resetSelection();
		m_widget2d->getEventHandle(m_widget2d->getEventHandleType())->resetSelection();
		updateUiByParam();
	} catch (std::exception e)
	{
		std::cout << e.what() << std::endl;
	} catch (...)
	{
		std::cout << "on_actionPrev_triggered(): unknown exception!" << std::endl;
	}
}

void ClothDesigner::on_actionNext_triggered()
{
	try
	{
		g_dataholder.m_historyStack->stepForward();
		m_widget2d->updateGL();
		m_widget3d->updateGL();
		m_widget3d->getEventHandle(m_widget3d->getEventHandleType())->resetSelection();
		m_widget2d->getEventHandle(m_widget2d->getEventHandleType())->resetSelection();
		updateUiByParam();
	} catch (std::exception e)
	{
		std::cout << e.what() << std::endl;
	} catch (...)
	{
		std::cout << "on_actionNext_triggered(): unknown exception!" << std::endl;
	}
}

//////right dock///////////////////////////////////////////////////////////////////////////////
void ClothDesigner::updateUiByParam()
{
	try
	{
		//// simulation param
		auto param = g_dataholder.m_clothManager->getSimulationParam();
		ui.sbSparamAirDamp->setValue(param.air_damping);
		ui.sbSparamBendStiff->setValue(param.bending_k);
		ui.sbSparamControlStiff->setValue(param.control_mag);
		ui.sbSparamInnerIter->setValue(param.inner_iter);
		ui.sbSparamLapDampIter->setValue(param.lap_damping);
		ui.sbSparamOuterIter->setValue(param.out_iter);
		ui.sbSparamRho->setValue(param.rho);
		ui.sbSparamSpringStiff->setValue(param.spring_k_raw);
		ui.sbSparamStitchStiff->setValue(param.stitch_k_raw);
		ui.sbSparamStitchSpeed->setValue(param.stitch_ratio);
		ui.sbSparamStitchBend->setValue(param.stitch_bending_k);
		ui.sbSparamTimeStepInv->setValue(1./param.time_step);
		ui.sbSparamUnderRelax->setValue(param.under_relax);
		ui.sbSparamGravityX->setValue(param.gravity[0]);
		ui.sbSparamGravityY->setValue(param.gravity[1]);
		ui.sbSparamGravityZ->setValue(param.gravity[2]);

		///design params
		auto dparam = g_dataholder.m_clothManager->getClothDesignParam();
		ui.sbDparamTriangleSize->setValue(dparam.triangulateThre * 1000);

		// if only one piece selected, we update the piece param
		// this is due to the current UI do not display all piece params, but only the selected one
		std::set<ldp::ClothPiece*> selectedPieces;
		for (int i = 0; i < g_dataholder.m_clothManager->numClothPieces(); i++)
		{
			auto piece = g_dataholder.m_clothManager->clothPiece(i);
			if (piece->graphPanel().isSelected())
				selectedPieces.insert(piece);
		}
		if (selectedPieces.size() == 1)
		{
			auto piece = *selectedPieces.begin();
			ui.dbPieceBendMult->setValue(piece->param().bending_k_mult);
			ui.dbPieceOutgoDist->setValue(piece->param().piece_outgo_dist);
		}

		//// smpl body param
		updateSmplUI();
	} catch (std::exception e)
	{
		std::cout << e.what() << std::endl;
	} catch (...)
	{
		std::cout << "unknown error" << std::endl;
	}
}

void ClothDesigner::on_pbResetSimulation_clicked()
{
	try
	{
		g_dataholder.m_clothManager->simulationInit();
		m_widget3d->updateGL();
	} catch (std::exception e)
	{
		std::cout << e.what() << std::endl;
	} catch (...)
	{
		std::cout << "unknown error" << std::endl;
	}
}

void ClothDesigner::on_sbSparamOuterIter_valueChanged(int v)
{
	try
	{
		auto param = g_dataholder.m_clothManager->getSimulationParam();
		param.out_iter = v;
		g_dataholder.m_clothManager->setSimulationParam(param);
	} catch (std::exception e)
	{
		std::cout << e.what() << std::endl;
	} catch (...)
	{
		std::cout << "unknown error" << std::endl;
	}
}

void ClothDesigner::on_sbSparamInnerIter_valueChanged(int v)
{
	try
	{
		auto param = g_dataholder.m_clothManager->getSimulationParam();
		param.inner_iter = v;
		g_dataholder.m_clothManager->setSimulationParam(param);
	} catch (std::exception e)
	{
		std::cout << e.what() << std::endl;
	} catch (...)
	{
		std::cout << "unknown error" << std::endl;
	}
}

void ClothDesigner::on_sbSparamTimeStepInv_valueChanged(int v)
{
	try
	{
		auto param = g_dataholder.m_clothManager->getSimulationParam();
		param.time_step = 1./v;
		g_dataholder.m_clothManager->setSimulationParam(param);
	} catch (std::exception e)
	{
		std::cout << e.what() << std::endl;
	} catch (...)
	{
		std::cout << "unknown error" << std::endl;
	}
}

void ClothDesigner::on_sbSparamLapDampIter_valueChanged(int v)
{
	try
	{
		auto param = g_dataholder.m_clothManager->getSimulationParam();
		param.lap_damping = v;
		g_dataholder.m_clothManager->setSimulationParam(param);
	} catch (std::exception e)
	{
		std::cout << e.what() << std::endl;
	} catch (...)
	{
		std::cout << "unknown error" << std::endl;
	}
}

void ClothDesigner::on_sbSparamAirDamp_valueChanged(double v)
{
	try
	{
		auto param = g_dataholder.m_clothManager->getSimulationParam();
		param.air_damping = v;
		g_dataholder.m_clothManager->setSimulationParam(param);
	} catch (std::exception e)
	{
		std::cout << e.what() << std::endl;
	} catch (...)
	{
		std::cout << "unknown error" << std::endl;
	}
}

void ClothDesigner::on_sbSparamControlStiff_valueChanged(double v)
{
	try
	{
		auto param = g_dataholder.m_clothManager->getSimulationParam();
		param.control_mag = v;
		g_dataholder.m_clothManager->setSimulationParam(param);
	} catch (std::exception e)
	{
		std::cout << e.what() << std::endl;
	} catch (...)
	{
		std::cout << "unknown error" << std::endl;
	}
}

void ClothDesigner::on_sbSparamRho_valueChanged(double v)
{
	try
	{
		auto param = g_dataholder.m_clothManager->getSimulationParam();
		param.rho = v;
		g_dataholder.m_clothManager->setSimulationParam(param);
	} catch (std::exception e)
	{
		std::cout << e.what() << std::endl;
	} catch (...)
	{
		std::cout << "unknown error" << std::endl;
	}
}

void ClothDesigner::on_sbSparamUnderRelax_valueChanged(double v)
{
	try
	{
		auto param = g_dataholder.m_clothManager->getSimulationParam();
		param.under_relax = v;
		g_dataholder.m_clothManager->setSimulationParam(param);
	} catch (std::exception e)
	{
		std::cout << e.what() << std::endl;
	} catch (...)
	{
		std::cout << "unknown error" << std::endl;
	}
}

void ClothDesigner::on_sbSparamSpringStiff_valueChanged(double v)
{
	try
	{
		auto param = g_dataholder.m_clothManager->getSimulationParam();
		param.spring_k_raw = v;
		g_dataholder.m_clothManager->setSimulationParam(param);
	} catch (std::exception e)
	{
		std::cout << e.what() << std::endl;
	} catch (...)
	{
		std::cout << "unknown error" << std::endl;
	}
}

void ClothDesigner::on_sbSparamBendStiff_valueChanged(double v)
{
	try
	{
		auto param = g_dataholder.m_clothManager->getSimulationParam();
		param.bending_k = v;
		g_dataholder.m_clothManager->setSimulationParam(param);
	} catch (std::exception e)
	{
		std::cout << e.what() << std::endl;
	} catch (...)
	{
		std::cout << "unknown error" << std::endl;
	}
}

void ClothDesigner::on_sbSparamStitchStiff_valueChanged(double v)
{
	try
	{
		auto param = g_dataholder.m_clothManager->getSimulationParam();
		param.stitch_k_raw = v;
		g_dataholder.m_clothManager->setSimulationParam(param);
	} catch (std::exception e)
	{
		std::cout << e.what() << std::endl;
	} catch (...)
	{
		std::cout << "unknown error" << std::endl;
	}
}

void ClothDesigner::on_sbSparamStitchSpeed_valueChanged(double v)
{
	try
	{
		auto param = g_dataholder.m_clothManager->getSimulationParam();
		param.stitch_ratio = v;
		g_dataholder.m_clothManager->setSimulationParam(param);
	} catch (std::exception e)
	{
		std::cout << e.what() << std::endl;
	} catch (...)
	{
		std::cout << "unknown error" << std::endl;
	}
}

void ClothDesigner::on_sbSparamStitchBend_valueChanged(double v)
{
	try
	{
		auto param = g_dataholder.m_clothManager->getSimulationParam();
		param.stitch_bending_k = v;
		g_dataholder.m_clothManager->setSimulationParam(param);
	} catch (std::exception e)
	{
		std::cout << e.what() << std::endl;
	} catch (...)
	{
		std::cout << "unknown error" << std::endl;
	}
}

void ClothDesigner::on_sbSparamGravityX_valueChanged(double v)
{
	try
	{
		auto param = g_dataholder.m_clothManager->getSimulationParam();
		param.gravity[0] = v;
		g_dataholder.m_clothManager->setSimulationParam(param);
	} catch (std::exception e)
	{
		std::cout << e.what() << std::endl;
	} catch (...)
	{
		std::cout << "unknown error" << std::endl;
	}
}

void ClothDesigner::on_sbSparamGravityY_valueChanged(double v)
{
	try
	{
		auto param = g_dataholder.m_clothManager->getSimulationParam();
		param.gravity[1] = v;
		g_dataholder.m_clothManager->setSimulationParam(param);
	} catch (std::exception e)
	{
		std::cout << e.what() << std::endl;
	} catch (...)
	{
		std::cout << "unknown error" << std::endl;
	}
}

void ClothDesigner::on_sbSparamGravityZ_valueChanged(double v)
{
	try
	{
		auto param = g_dataholder.m_clothManager->getSimulationParam();
		param.gravity[2] = v;
		g_dataholder.m_clothManager->setSimulationParam(param);
	} catch (std::exception e)
	{
		std::cout << e.what() << std::endl;
	} catch (...)
	{
		std::cout << "unknown error" << std::endl;
	}
}

/////main tool bars////////////////////////////////////////////////////////////////////////////
void ClothDesigner::resizeEvent(QResizeEvent* ev)
{

}

void ClothDesigner::init3dActions()
{
	// add buttons
	for (size_t i = (size_t)Abstract3dEventHandle::ProcessorTypeGeneral + 1;
		i < (size_t)Abstract3dEventHandle::ProcessorTypeEnd; i++)
	{
		auto type = Abstract3dEventHandle::ProcessorType(i);
		add3dButton(type);
	}
	m_widget3d->setEventHandleType(Abstract3dEventHandle::ProcessorTypeSelect);
}

void ClothDesigner::add3dButton(Abstract3dEventHandle::ProcessorType type)
{
	auto handle = m_widget3d->getEventHandle(type);
	auto colorStr = QString("background-color: rgb(73, 73, 73)");
	QIcon icon;
	icon.addFile(handle->iconFile());
	QAction* action = new QAction(icon, QString().sprintf("%d", type), ui.mainToolBar);
	action->setToolTip(handle->toolTips());
	ui.mainToolBar->addAction(action);
}

void ClothDesigner::init2dActions()
{
	ui.mainToolBar->addSeparator();
	// add buttons
	for (size_t i = (size_t)Abstract2dEventHandle::ProcessorTypeGeneral + 1;
		i < (size_t)Abstract2dEventHandle::ProcessorTypeEnd; i++)
	{
		auto type = Abstract2dEventHandle::ProcessorType(i);
		add2dButton(type);
	}
	m_widget2d->setEventHandleType(Abstract2dEventHandle::ProcessorTypeEditPattern);
}

void ClothDesigner::add2dButton(Abstract2dEventHandle::ProcessorType type)
{
	auto handle = m_widget2d->getEventHandle(type);
	auto colorStr = QString("background-color: rgb(73, 73, 73)");
	QIcon icon;
	icon.addFile(handle->iconFile(), QSize(), QIcon::Active);
	icon.addFile(handle->iconFile(), QSize(), QIcon::Selected);
	icon.addFile(handle->inactiveIconFile(), QSize(), QIcon::Normal);
	QAction* action = new QAction(icon, QString().sprintf("%d", 
		type+Abstract3dEventHandle::ProcessorTypeEnd), ui.mainToolBar);
	action->setToolTip(handle->toolTips());
	ui.mainToolBar->addAction(action);
}

void ClothDesigner::on_mainToolBar_actionTriggered(QAction* action)
{
	int id = action->text().toInt();
	if (id < Abstract3dEventHandle::ProcessorTypeEnd)
	{
		Abstract3dEventHandle::ProcessorType type = (Abstract3dEventHandle::ProcessorType)id;
		m_widget3d->setEventHandleType(type);
	}
	else
	{
		Abstract2dEventHandle::ProcessorType type = (Abstract2dEventHandle::ProcessorType)
			(id-Abstract3dEventHandle::ProcessorTypeEnd);
		m_widget2d->setEventHandleType(type);
	}
}

/////lower dock////////////////////////////////////////////////////////////////////////////////
void ClothDesigner::on_pbFlipPolygon_clicked()
{
	try
	{
		auto& manager = g_dataholder.m_clothManager;
		bool changed = false;
		for (size_t iPiece = 0; iPiece < manager->numClothPieces(); iPiece++)
		{
			auto piece = manager->clothPiece(iPiece);
			auto& panel = piece->graphPanel();
			if (panel.isSelected())
			{
				piece->transformInfo().flipNormal();
				piece->mesh2d().flipNormals();
				piece->mesh3d().flipNormals();
				piece->mesh3dInit().flipNormals();
				changed = true;
			}
		} // end for iPiece
		if (changed)
		{
			//manager->updateCloths3dMeshBy2d();
			m_widget2d->updateGL();
			m_widget3d->updateGL();
			pushHistory("flip polygons", ldp::HistoryStack::TypeGeneral);
		}
	} catch (std::exception e)
	{
		std::cout << e.what() << std::endl;
	} catch (...)
	{
		std::cout << "unknown error" << std::endl;
	}
}

void ClothDesigner::on_sbDparamTriangleSize_valueChanged(double v)
{
	try
	{
		auto& manager = g_dataholder.m_clothManager;
		auto param = manager->getClothDesignParam();
		param.triangulateThre = v/1000;
		manager->setClothDesignParam(param);
		manager->triangulate();
		manager->setSimulationMode(ldp::SimulationMode::SimulationPause);
		m_widget2d->updateGL();
		m_widget3d->updateGL();
		pushHistory(QString().sprintf("triangulate: %f", v), ldp::HistoryStack::TypeGeneral);	
	} catch (std::exception e)
	{
		std::cout << e.what() << std::endl;
	} catch (...)
	{
		std::cout << "unknown error" << std::endl;
	}
}

void ClothDesigner::on_dbPieceOutgoDist_valueChanged(double v)
{
	for (int ipiece = 0; ipiece < g_dataholder.m_clothManager->numClothPieces(); ipiece++)
	{
		auto piece = g_dataholder.m_clothManager->clothPiece(ipiece);
		auto& panel = piece->graphPanel();
		if (!panel.isSelected())
			continue;
		auto param = piece->param();
		param.piece_outgo_dist = v;
		g_dataholder.m_clothManager->setPieceParam(piece, param);
		printf("%s, outgo_dist = %f meters\n", piece->getName().c_str(), v);
	}
}

void ClothDesigner::on_dbPieceBendMult_valueChanged(double v)
{
	for (int ipiece = 0; ipiece < g_dataholder.m_clothManager->numClothPieces(); ipiece++)
	{
		auto piece = g_dataholder.m_clothManager->clothPiece(ipiece);
		auto& panel = piece->graphPanel();
		if (!panel.isSelected())
			continue;
		auto param = piece->param();
		param.bending_k_mult = v;
		g_dataholder.m_clothManager->setPieceParam(piece, param);
		printf("%s, bend_k_mult = %f\n", piece->getName().c_str(), v);
	}
}

void ClothDesigner::on_pbMirrorSelected_clicked()
{
	try
	{
		auto& manager = g_dataholder.m_clothManager;
		if (manager->mirrorSelectedPanel())
		{
			manager->triangulate();
			m_widget2d->updateGL();
			m_widget3d->updateGL();
			pushHistory(QString().sprintf("mirror selected"), ldp::HistoryStack::TypeGeneral);
		}
	} catch (std::exception e)
	{
		std::cout << e.what() << std::endl;
	} catch (...)
	{
		std::cout << "unknown error" << std::endl;
	}
}

void ClothDesigner::on_pbCopySelected_clicked()
{
	try
	{
		auto& manager = g_dataholder.m_clothManager;
		if (manager->copySelectedPanel())
		{
			manager->triangulate();
			m_widget2d->updateGL();
			m_widget3d->updateGL();
			pushHistory(QString().sprintf("mirror selected"), ldp::HistoryStack::TypeGeneral);
		}
	} catch (std::exception e)
	{
		std::cout << e.what() << std::endl;
	} catch (...)
	{
		std::cout << "unknown error" << std::endl;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////
void ClothDesigner::setupSmplUI()
{
	m_smplShapeSliders.resize(10);
	ui.gpSmplBodyCoeffs->setLayout(new QGridLayout());
	for (size_t i = 0; i < m_smplShapeSliders.size(); i++)
	{
		m_smplShapeSliders[i].reset(new QSlider(Qt::Orientation::Horizontal, this));
		m_smplShapeSliders[i]->setMinimum(-500);
		m_smplShapeSliders[i]->setMaximum(500);
		m_smplShapeSliders[i]->setValue(0);
		ui.gpSmplBodyCoeffs->layout()->addWidget(m_smplShapeSliders[i].data());
		connect(m_smplShapeSliders[i].data(), SIGNAL(valueChanged(int)), SLOT(onSmplShapeSlidersValueChanged(int)));
	}
}

void ClothDesigner::updateSmplUI()
{
	try
	{
		m_sliderEnableSmplUpdate = false;
		auto smpl = g_dataholder.m_clothManager->bodySmplManager();
		if (smpl == nullptr)
		{
			ui.gpSmplBodyCoeffs->setEnabled(false);
			m_sliderEnableSmplUpdate = true;
			return;
		}
		if (m_smplShapeSliders.size() != smpl->numShapes())
			throw std::exception("smpl ui update: size not matched!");
		ui.gpSmplBodyCoeffs->setEnabled(true);
		for (size_t i = 0; i < m_smplShapeSliders.size(); i++)
			m_smplShapeSliders[i]->setValue(std::lroundf(smpl->getCurShapeCoef(i) * 100));
		m_sliderEnableSmplUpdate = true;
	} catch (std::exception e)
	{
		m_sliderEnableSmplUpdate = true;
		std::cout << e.what() << std::endl;
	} catch (...)
	{
		m_sliderEnableSmplUpdate = true;
		std::cout << "unknown error" << std::endl;
	}
}

void ClothDesigner::onSmplShapeSlidersValueChanged(int v)
{
	try
	{
		auto smpl = g_dataholder.m_clothManager->bodySmplManager();
		if (smpl == nullptr)
			return;
		if (m_smplShapeSliders.size() != smpl->numShapes())
			throw std::exception("smpl ui update: size not matched!");
		std::vector<float> shapes(smpl->numShapes(), 0.f);
		for (size_t i = 0; i < m_smplShapeSliders.size(); i++)
		{
			float val = m_smplShapeSliders[i]->value() / 100.f;
			shapes[i] = val;
			m_smplShapeSliders[i]->setToolTip(QString().sprintf("%f", val));
		}
		if (m_sliderEnableSmplUpdate)
		{
			smpl->setPoseShapeVals(nullptr, &shapes);
			g_dataholder.m_clothManager->simulationInit();
			g_dataholder.m_clothManager->updateSmplBody();
			m_widget3d->updateGL();
		}
	} catch (std::exception e)
	{
		std::cout << e.what() << std::endl;
	} catch (...)
	{
		std::cout << "unknown error" << std::endl;
	}
}

void ClothDesigner::on_pbSaveSmplCoeffs_clicked()
{
	try
	{
		auto smpl = g_dataholder.m_clothManager->bodySmplManager();
		if (smpl == nullptr)
			return;
		QString name = QFileDialog::getSaveFileName(this, "save shape coef", 
			g_dataholder.m_lastSmplShapeCoeffDir.c_str(), "*.shape.txt");
		if (name.isEmpty())
			return;
		if (!name.endsWith(".shape.txt"))
			name.append(".shape.txt");
		smpl->saveShapeCoeffs(name.toStdString());
	} catch (std::exception e)
	{
		std::cout << e.what() << std::endl;
	} catch (...)
	{
		std::cout << "unknown error" << std::endl;
	}
}

void ClothDesigner::on_pbLoadSmplCoeffs_clicked()
{
	try
	{
		auto smpl = g_dataholder.m_clothManager->bodySmplManager();
		if (smpl == nullptr)
			return;
		QString name = QFileDialog::getOpenFileName(this, "open shape/pose coef",
			g_dataholder.m_lastSmplShapeCoeffDir.c_str(), "*.txt");
		if (name.isEmpty())
			return;
		g_dataholder.m_lastSmplShapeCoeffDir = name.toStdString();
		g_dataholder.saveLastDirs();
		if (name.toLower().endsWith(".shape.txt"))
			smpl->loadShapeCoeffs(name.toStdString());
		else if (name.toLower().endsWith(".pose.txt"))
			smpl->loadPoseCoeffs(name.toStdString());
		if (m_smplShapeSliders.size() != smpl->numShapes())
			throw std::exception("smpl ui update: size not matched!");

		updateSmplUI();

		g_dataholder.m_clothManager->updateSmplBody();
		m_widget3d->updateGL();
	} catch (std::exception e)
	{
		std::cout << e.what() << std::endl;
	} catch (...)
	{
		std::cout << "unknown error" << std::endl;
	} 
}

void ClothDesigner::on_pbResetSmplCoeffs_clicked()
{
	try
	{
		auto smpl = g_dataholder.m_clothManager->bodySmplManager();
		if (smpl == nullptr)
			return;
		if (m_smplShapeSliders.size() != smpl->numShapes())
			throw std::exception("smpl ui update: size not matched!");

		std::vector<float> shapes(smpl->numShapes(), 0.f);
		m_sliderEnableSmplUpdate = false;
		for (size_t i = 0; i < m_smplShapeSliders.size(); i++)
		{
			m_smplShapeSliders[i]->setValue(0);
			m_smplShapeSliders[i]->setToolTip("0");
		}
		m_sliderEnableSmplUpdate = true;

		std::vector<float> poses(smpl->numPoses()*smpl->numVarEachPose(), 0.f);
		smpl->setPoseShapeVals(&poses, &shapes);
		g_dataholder.m_clothManager->updateSmplBody();
		m_widget3d->updateGL();
	} catch (std::exception e)
	{
		std::cout << e.what() << std::endl;
	} catch (...)
	{
		std::cout << "unknown error" << std::endl;
	}
}

void ClothDesigner::on_pbBindClothesToSmpl_clicked()
{
	try
	{
		auto smpl = g_dataholder.m_clothManager->bodySmplManager();
		if (smpl == nullptr)
			return;
		g_dataholder.m_clothManager->bindClothesToSmplJoints();
		g_dataholder.m_clothManager->updateClothBySmplJoints();
		m_widget3d->updateGL();
	} catch (std::exception e)
	{
		std::cout << e.what() << std::endl;
	} catch (...)
	{
		std::cout << "unknown error" << std::endl;
	}
}