#include "clothdesigner.h"
#include <QGridLayout>
#include "global_data_holder.h"

#include "viewer2d.h"
#include "viewer3d.h"

ClothDesigner::ClothDesigner(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	
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
	m_widget3d->init(g_dataholder.m_clothManager.get(), this);
	m_widget2d->init(g_dataholder.m_clothManager.get(), this);

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
		g_dataholder.m_clothManager->simulationUpdate();
		m_widget3d->updateGL();
	}

	if (ev->timerId() == m_fpsTimer)
		setWindowTitle(QString().sprintf("fps: %f", g_dataholder.m_clothManager->getFps()));
}

void ClothDesigner::on_actionLoad_svg_triggered()
{
	try
	{
		g_dataholder.debug_5();
		g_dataholder.m_clothManager->simulationInit();

		m_widget3d->init(g_dataholder.m_clothManager.get(), this);
		m_widget2d->init(g_dataholder.m_clothManager.get(), this);
		m_widget2d->updateGL();
		m_widget3d->updateGL();
	} catch (std::exception e)
	{
		std::cout << e.what() << std::endl;
	}
}

void ClothDesigner::updateUiByParam()
{
	try
	{
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
		ui.sbSparamTimeStepInv->setValue(1./param.time_step);
		ui.sbSparamUnderRelax->setValue(param.under_relax);
		ui.sbSparamGravityX->setValue(param.gravity[0]);
		ui.sbSparamGravityY->setValue(param.gravity[1]);
		ui.sbSparamGravityZ->setValue(param.gravity[2]);
	} catch (std::exception e)
	{
		std::cout << e.what() << std::endl;
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
	}
}

/////////////////////////////////////////////////////////////////////////////////
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