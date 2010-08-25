/**
 ******************************************************************************
 *
 * @file       configservowidget.cpp
 * @author     E. Lafargue & The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ConfigPlugin Config Plugin
 * @{
 * @brief Servo input/output configuration panel for the config gadget
 *****************************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
#include "configservowidget.h"

#include <QDebug>
#include <QStringList>
#include <QtGui/QWidget>
#include <QtGui/QTextEdit>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>



ConfigServoWidget::ConfigServoWidget(QWidget *parent) : ConfigTaskWidget(parent)
{
    m_config = new Ui_SettingsWidget();
    m_config->setupUi(this);

    // Fill in the dropdown menus for the channel RC Input assignement.
    QStringList channelsList;
    channelsList << "None" << "Roll" << "Pitch" << "Yaw" << "Throttle" << "FlightMode";
    m_config->ch0Assign->addItems(channelsList);
    m_config->ch1Assign->addItems(channelsList);
    m_config->ch2Assign->addItems(channelsList);
    m_config->ch3Assign->addItems(channelsList);
    m_config->ch4Assign->addItems(channelsList);
    m_config->ch5Assign->addItems(channelsList);
    m_config->ch6Assign->addItems(channelsList);
    m_config->ch7Assign->addItems(channelsList);

    // Now connect the widget to the ManualControlCommand / Channel UAVObject
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();

    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(objManager->getObject(QString("ManualControlCommand")));
    connect(obj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(updateChannels(UAVObject*)));

    // Get the receiver types supported by OpenPilot and fill the corresponding
    // dropdown menu:
    obj = dynamic_cast<UAVDataObject*>(objManager->getObject(QString("ManualControlSettings")));
    QString fieldName = QString("InputMode");
    UAVObjectField *field = obj->getField(fieldName);
    m_config->receiverType->addItems(field->getOptions());

    // And for the channel output assignement options
    m_config->ch0Output->addItem("None");
    m_config->ch1Output->addItem("None");
    m_config->ch2Output->addItem("None");
    m_config->ch3Output->addItem("None");
    m_config->ch4Output->addItem("None");
    m_config->ch5Output->addItem("None");
    m_config->ch6Output->addItem("None");
    m_config->ch7Output->addItem("None");

    obj = dynamic_cast<UAVDataObject*>(objManager->getObject(QString("ActuatorSettings")));
    QList<UAVObjectField*> fieldList = obj->getFields();
    foreach (UAVObjectField* field, fieldList) {
        // NOTE: we assume that all options in ActuatorSettings are a channel assignement
        // except for the options called "ChannelXXX"
        if (field->getUnits().contains("channel")) {
//        if (!field->getName().contains("Channel")) {
            m_config->ch0Output->addItem(field->getName());
            m_config->ch1Output->addItem(field->getName());
            m_config->ch2Output->addItem(field->getName());
            m_config->ch3Output->addItem(field->getName());
            m_config->ch4Output->addItem(field->getName());
            m_config->ch5Output->addItem(field->getName());
            m_config->ch6Output->addItem(field->getName());
            m_config->ch7Output->addItem(field->getName());
        }
    }

    connect(m_config->ch0OutMin, SIGNAL(editingFinished()), this, SLOT(setch0OutRange()));
    connect(m_config->ch1OutMin, SIGNAL(editingFinished()), this, SLOT(setch1OutRange()));
    connect(m_config->ch2OutMin, SIGNAL(editingFinished()), this, SLOT(setch2OutRange()));
    connect(m_config->ch3OutMin, SIGNAL(editingFinished()), this, SLOT(setch3OutRange()));
    connect(m_config->ch4OutMin, SIGNAL(editingFinished()), this, SLOT(setch4OutRange()));
    connect(m_config->ch5OutMin, SIGNAL(editingFinished()), this, SLOT(setch5OutRange()));
    connect(m_config->ch6OutMin, SIGNAL(editingFinished()), this, SLOT(setch6OutRange()));
    connect(m_config->ch7OutMin, SIGNAL(editingFinished()), this, SLOT(setch7OutRange()));

    connect(m_config->ch0OutMax, SIGNAL(editingFinished()), this, SLOT(setch0OutRange()));
    connect(m_config->ch1OutMax, SIGNAL(editingFinished()), this, SLOT(setch1OutRange()));
    connect(m_config->ch2OutMax, SIGNAL(editingFinished()), this, SLOT(setch2OutRange()));
    connect(m_config->ch3OutMax, SIGNAL(editingFinished()), this, SLOT(setch3OutRange()));
    connect(m_config->ch4OutMax, SIGNAL(editingFinished()), this, SLOT(setch4OutRange()));
    connect(m_config->ch5OutMax, SIGNAL(editingFinished()), this, SLOT(setch5OutRange()));
    connect(m_config->ch6OutMax, SIGNAL(editingFinished()), this, SLOT(setch6OutRange()));
    connect(m_config->ch7OutMax, SIGNAL(editingFinished()), this, SLOT(setch7OutRange()));

    connect(m_config->channelOutTest, SIGNAL(toggled(bool)), this, SLOT(runChannelTests(bool)));

    requestRCInputUpdate();
    requestRCOutputUpdate();

    connect(m_config->saveRCInputToSD, SIGNAL(clicked()), this, SLOT(saveRCInputObject()));
    connect(m_config->saveRCInputToRAM, SIGNAL(clicked()), this, SLOT(sendRCInputUpdate()));
    connect(m_config->getRCInputCurrent, SIGNAL(clicked()), this, SLOT(requestRCInputUpdate()));

    connect(m_config->saveRCOutputToSD, SIGNAL(clicked()), this, SLOT(saveRCOutputObject()));
    connect(m_config->saveRCOutputToRAM, SIGNAL(clicked()), this, SLOT(sendRCOutputUpdate()));
    connect(m_config->getRCOutputCurrent, SIGNAL(clicked()), this, SLOT(requestRCOutputUpdate()));

    // Now connect the channel out sliders to our signal to send updates in test mode
    connect(m_config->ch0OutSlider, SIGNAL(valueChanged(int)), this, SLOT(sendChannelTest(int)));
    connect(m_config->ch1OutSlider, SIGNAL(valueChanged(int)), this, SLOT(sendChannelTest(int)));
    connect(m_config->ch2OutSlider, SIGNAL(valueChanged(int)), this, SLOT(sendChannelTest(int)));
    connect(m_config->ch3OutSlider, SIGNAL(valueChanged(int)), this, SLOT(sendChannelTest(int)));
    connect(m_config->ch4OutSlider, SIGNAL(valueChanged(int)), this, SLOT(sendChannelTest(int)));
    connect(m_config->ch5OutSlider, SIGNAL(valueChanged(int)), this, SLOT(sendChannelTest(int)));
    connect(m_config->ch6OutSlider, SIGNAL(valueChanged(int)), this, SLOT(sendChannelTest(int)));
    connect(m_config->ch7OutSlider, SIGNAL(valueChanged(int)), this, SLOT(sendChannelTest(int)));

    firstUpdate = true;

}

ConfigServoWidget::~ConfigServoWidget()
{
   // Do nothing
}


/**
  Sends the channel value to the UAV to move the servo.
  Returns immediately if we are not in testing mode
  */
void ConfigServoWidget::sendChannelTest(int value)
{
    if (!m_config->channelOutTest->isChecked())
        return;

    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();

    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(objManager->getObject(QString("ActuatorCommand")));

    QObject *ob = QObject::sender();
    QStringList channelsList;
    channelsList << "ch0OutSlider" << "ch1OutSlider" << "ch2OutSlider" << "ch3OutSlider" << "ch4OutSlider"
            << "ch5OutSlider" << "ch6OutSlider" << "ch7OutSlider";
    int idx = channelsList.indexOf(QRegExp(ob->objectName()));
    UAVObjectField * channel = obj->getField("Channel");
    channel->setValue(value,idx);
    obj->updated();

}

/**
  Toggles the channel testing mode by making the GCS take over
  the ActuatorCommand objects
  */
void ConfigServoWidget::runChannelTests(bool state)
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();

    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(objManager->getObject(QString("ActuatorCommand")));
    UAVObject::Metadata mdata = obj->getMetadata();
    if (state)
    {
        accInitialData = mdata;
        mdata.flightAccess = UAVObject::ACCESS_READONLY;
        mdata.flightTelemetryUpdateMode = UAVObject::UPDATEMODE_ONCHANGE;
        mdata.gcsTelemetryAcked = false;
        mdata.gcsTelemetryUpdateMode = UAVObject::UPDATEMODE_ONCHANGE;
        mdata.gcsTelemetryUpdatePeriod = 100;
    }
    else
    {
        mdata = accInitialData; // Restore metadata
    }
    obj->setMetadata(mdata);

}



/********************************
  *  Output settings
  *******************************/

/**
  Request the current config from the board (RC Output)
  */
void ConfigServoWidget::requestRCOutputUpdate()
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();

    // Get the Airframe type from the system settings:
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(objManager->getObject(QString("SystemSettings")));
    Q_ASSERT(obj);
    obj->requestUpdate();
    UAVObjectField *field = obj->getField(QString("AirframeType"));
    m_config->aircraftType->setText(QString("Aircraft type: ") + field->getValue().toString());

    // Reset all channel assignements:
    m_config->ch0Output->setCurrentIndex(0);
    m_config->ch1Output->setCurrentIndex(0);
    m_config->ch2Output->setCurrentIndex(0);
    m_config->ch3Output->setCurrentIndex(0);
    m_config->ch4Output->setCurrentIndex(0);
    m_config->ch5Output->setCurrentIndex(0);
    m_config->ch6Output->setCurrentIndex(0);
    m_config->ch7Output->setCurrentIndex(0);

    // Get the channel assignements:
    obj = dynamic_cast<UAVDataObject*>(objManager->getObject(QString("ActuatorSettings")));
    Q_ASSERT(obj);
    obj->requestUpdate();
    QList<UAVObjectField*> fieldList = obj->getFields();
    foreach (UAVObjectField* field, fieldList) {
        if (!field->getName().contains("Channel")) {
            assignOutputChannel(obj,field,field->getName());
        }
    }

    // Get Output rates for both banks
    field = obj->getField(QString("ChannelUpdateFreq"));
    m_config->outputRate1->setValue(field->getValue(0).toInt());
    m_config->outputRate2->setValue(field->getValue(1).toInt());

    // Get Channel ranges:
    field = obj->getField(QString("ChannelMin"));
    m_config->ch0OutMin->setValue(field->getValue(0).toInt());
    m_config->ch0OutSlider->setMinimum(field->getValue(0).toInt());
    m_config->ch1OutMin->setValue(field->getValue(1).toInt());
    m_config->ch1OutSlider->setMinimum(field->getValue(1).toInt());
    m_config->ch2OutMin->setValue(field->getValue(2).toInt());
    m_config->ch2OutSlider->setMinimum(field->getValue(2).toInt());
    m_config->ch3OutMin->setValue(field->getValue(3).toInt());
    m_config->ch3OutSlider->setMinimum(field->getValue(3).toInt());
    m_config->ch4OutMin->setValue(field->getValue(4).toInt());
    m_config->ch4OutSlider->setMinimum(field->getValue(4).toInt());
    m_config->ch5OutMin->setValue(field->getValue(5).toInt());
    m_config->ch5OutSlider->setMinimum(field->getValue(5).toInt());
    m_config->ch6OutMin->setValue(field->getValue(6).toInt());
    m_config->ch6OutSlider->setMinimum(field->getValue(6).toInt());
    m_config->ch7OutMin->setValue(field->getValue(7).toInt());
    m_config->ch7OutSlider->setMinimum(field->getValue(7).toInt());

    field = obj->getField(QString("ChannelMax"));
    m_config->ch0OutMax->setValue(field->getValue(0).toInt());
    m_config->ch0OutSlider->setMaximum(field->getValue(0).toInt());
    m_config->ch1OutMax->setValue(field->getValue(1).toInt());
    m_config->ch1OutSlider->setMaximum(field->getValue(1).toInt());
    m_config->ch2OutMax->setValue(field->getValue(2).toInt());
    m_config->ch2OutSlider->setMaximum(field->getValue(2).toInt());
    m_config->ch3OutMax->setValue(field->getValue(3).toInt());
    m_config->ch3OutSlider->setMaximum(field->getValue(3).toInt());
    m_config->ch4OutMax->setValue(field->getValue(4).toInt());
    m_config->ch4OutSlider->setMaximum(field->getValue(4).toInt());
    m_config->ch5OutMax->setValue(field->getValue(5).toInt());
    m_config->ch5OutSlider->setMaximum(field->getValue(5).toInt());
    m_config->ch6OutMax->setValue(field->getValue(6).toInt());
    m_config->ch6OutSlider->setMaximum(field->getValue(6).toInt());
    m_config->ch7OutMax->setValue(field->getValue(7).toInt());
    m_config->ch7OutSlider->setMaximum(field->getValue(7).toInt());

    field = obj->getField(QString("ChannelNeutral"));
    m_config->ch0OutSlider->setValue(field->getValue(0).toInt());
    m_config->ch1OutSlider->setValue(field->getValue(1).toInt());
    m_config->ch2OutSlider->setValue(field->getValue(2).toInt());
    m_config->ch3OutSlider->setValue(field->getValue(3).toInt());
    m_config->ch4OutSlider->setValue(field->getValue(4).toInt());
    m_config->ch5OutSlider->setValue(field->getValue(5).toInt());
    m_config->ch6OutSlider->setValue(field->getValue(6).toInt());
    m_config->ch7OutSlider->setValue(field->getValue(7).toInt());


}

/**
  * Sends the config to the board, without saving to the SD card (RC Output)
  */
void ConfigServoWidget::sendRCOutputUpdate()
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(objManager->getObject(QString("ActuatorSettings")));
    Q_ASSERT(obj);

    // Now send channel ranges:
    UAVObjectField * field = obj->getField(QString("ChannelMax"));
    field->setValue(m_config->ch0OutMax->value(),0);
    field->setValue(m_config->ch1OutMax->value(),1);
    field->setValue(m_config->ch2OutMax->value(),2);
    field->setValue(m_config->ch3OutMax->value(),3);
    field->setValue(m_config->ch4OutMax->value(),4);
    field->setValue(m_config->ch5OutMax->value(),5);
    field->setValue(m_config->ch6OutMax->value(),6);
    field->setValue(m_config->ch7OutMax->value(),7);

    field = obj->getField(QString("ChannelMin"));
    field->setValue(m_config->ch0OutMin->value(),0);
    field->setValue(m_config->ch1OutMin->value(),1);
    field->setValue(m_config->ch2OutMin->value(),2);
    field->setValue(m_config->ch3OutMin->value(),3);
    field->setValue(m_config->ch4OutMin->value(),4);
    field->setValue(m_config->ch5OutMin->value(),5);
    field->setValue(m_config->ch6OutMin->value(),6);
    field->setValue(m_config->ch7OutMin->value(),7);

    field = obj->getField(QString("ChannelNeutral"));
    field->setValue(m_config->ch0OutSlider->value(),0);
    field->setValue(m_config->ch1OutSlider->value(),1);
    field->setValue(m_config->ch2OutSlider->value(),2);
    field->setValue(m_config->ch3OutSlider->value(),3);
    field->setValue(m_config->ch4OutSlider->value(),4);
    field->setValue(m_config->ch5OutSlider->value(),5);
    field->setValue(m_config->ch6OutSlider->value(),6);
    field->setValue(m_config->ch7OutSlider->value(),7);

    field = obj->getField(QString("ChannelUpdateFreq"));
    field->setValue(m_config->outputRate1->value(),0);
    field->setValue(m_config->outputRate2->value(),1);

    // Set Actuator assignement for each channel:
    // Rule: if two channels have the same setting (which is wrong!) the higher channel
    // will get the setting.

    // First, reset all channel assignements:
    QList<UAVObjectField*> fieldList = obj->getFields();
    foreach (UAVObjectField* field, fieldList) {
        // NOTE: we assume that all options in ActuatorSettings are a channel assignement
        // except for the options called "ChannelXXX"
        if (field->getUnits().contains("channel")) {
            field->setValue(field->getOptions().last());
        }
    }

    if (m_config->ch0Output->currentIndex() != 0) {
        field = obj->getField(m_config->ch0Output->currentText());
        field->setValue(field->getOptions().at(0)); // -> This way we don't depend on channel naming convention
    }
    if (m_config->ch1Output->currentIndex() != 0) {
        field = obj->getField(m_config->ch1Output->currentText());
        field->setValue(field->getOptions().at(1)); // -> This way we don't depend on channel naming convention
    }
    if (m_config->ch2Output->currentIndex() != 0) {
        field = obj->getField(m_config->ch2Output->currentText());
        field->setValue(field->getOptions().at(2)); // -> This way we don't depend on channel naming convention
    }
    if (m_config->ch3Output->currentIndex() != 0) {
        field = obj->getField(m_config->ch3Output->currentText());
        field->setValue(field->getOptions().at(3)); // -> This way we don't depend on channel naming convention
    }
    if (m_config->ch4Output->currentIndex() != 0) {
        field = obj->getField(m_config->ch4Output->currentText());
        field->setValue(field->getOptions().at(4)); // -> This way we don't depend on channel naming convention
    }
    if (m_config->ch5Output->currentIndex() != 0) {
        field = obj->getField(m_config->ch5Output->currentText());
        field->setValue(field->getOptions().at(5)); // -> This way we don't depend on channel naming convention
    }
    if (m_config->ch6Output->currentIndex() != 0) {
        field = obj->getField(m_config->ch6Output->currentText());
        field->setValue(field->getOptions().at(6)); // -> This way we don't depend on channel naming convention
    }
    if (m_config->ch7Output->currentIndex() != 0) {
        field = obj->getField(m_config->ch7Output->currentText());
        field->setValue(field->getOptions().at(7)); // -> This way we don't depend on channel naming convention
    }

    // ... and send to the OP Board
    obj->updated();


}


/**
  Sends the config to the board and request saving into the SD card (RC Output)
  */
void ConfigServoWidget::saveRCOutputObject()
{
    // Send update so that the latest value is saved
    sendRCOutputUpdate();
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(objManager->getObject(QString("ActuatorSettings")));
    Q_ASSERT(obj);
    updateObjectPersistance(ObjectPersistence::OPERATION_SAVE, obj);

}


/**
  Sets the minimum/maximem value of the channel 0 to seven output sliders.
  Have to do it here because setMinimum is not a slot.

  One added trik: if the slider is at either its max or its min when the value
  is changed, then keep it on the max/min.
  */
void ConfigServoWidget::setch0OutRange()
{
    QSlider *slider = m_config->ch0OutSlider;
    int oldMini = slider->minimum();
    int oldMaxi = slider->maximum();
    slider->setRange(m_config->ch0OutMin->value(),
                                     m_config->ch0OutMax->value());
    if (slider->value()==oldMini)
        slider->setValue(slider->minimum());

    if (slider->value()==oldMaxi)
        slider->setValue(slider->maximum());
}
void ConfigServoWidget::setch1OutRange()
{
    QSlider *slider = m_config->ch1OutSlider;
    int oldMini = slider->minimum();
    int oldMaxi = slider->maximum();
    slider->setRange(m_config->ch1OutMin->value(),
                                     m_config->ch1OutMax->value());
    if (slider->value()==oldMini)
        slider->setValue(slider->minimum());

    if (slider->value()==oldMaxi)
        slider->setValue(slider->maximum());
}
void ConfigServoWidget::setch2OutRange()
{
    QSlider *slider = m_config->ch2OutSlider;
    int oldMini = slider->minimum();
    int oldMaxi = slider->maximum();
    slider->setRange(m_config->ch2OutMin->value(),
                                     m_config->ch2OutMax->value());
    if (slider->value()==oldMini)
        slider->setValue(slider->minimum());

    if (slider->value()==oldMaxi)
        slider->setValue(slider->maximum());
}
void ConfigServoWidget::setch3OutRange()
{
    QSlider *slider = m_config->ch3OutSlider;
    int oldMini = slider->minimum();
    int oldMaxi = slider->maximum();
    slider->setRange(m_config->ch3OutMin->value(),
                                     m_config->ch3OutMax->value());
    if (slider->value()==oldMini)
        slider->setValue(slider->minimum());

    if (slider->value()==oldMaxi)
        slider->setValue(slider->maximum());
}
void ConfigServoWidget::setch4OutRange()
{
    QSlider *slider = m_config->ch4OutSlider;
    int oldMini = slider->minimum();
    int oldMaxi = slider->maximum();
    slider->setRange(m_config->ch4OutMin->value(),
                                     m_config->ch4OutMax->value());
    if (slider->value()==oldMini)
        slider->setValue(slider->minimum());

    if (slider->value()==oldMaxi)
        slider->setValue(slider->maximum());
}
void ConfigServoWidget::setch5OutRange()
{
    QSlider *slider = m_config->ch5OutSlider;
    int oldMini = slider->minimum();
    int oldMaxi = slider->maximum();
    slider->setRange(m_config->ch5OutMin->value(),
                                     m_config->ch5OutMax->value());
    if (slider->value()==oldMini)
        slider->setValue(slider->minimum());

    if (slider->value()==oldMaxi)
        slider->setValue(slider->maximum());
}
void ConfigServoWidget::setch6OutRange()
{
    QSlider *slider = m_config->ch6OutSlider;
    int oldMini = slider->minimum();
    int oldMaxi = slider->maximum();
    slider->setRange(m_config->ch6OutMin->value(),
                                     m_config->ch6OutMax->value());
    if (slider->value()==oldMini)
        slider->setValue(slider->minimum());

    if (slider->value()==oldMaxi)
        slider->setValue(slider->maximum());
}
void ConfigServoWidget::setch7OutRange()
{
    QSlider *slider = m_config->ch7OutSlider;
    int oldMini = slider->minimum();
    int oldMaxi = slider->maximum();
    slider->setRange(m_config->ch7OutMin->value(),
                                     m_config->ch7OutMax->value());
    if (slider->value()==oldMini)
        slider->setValue(slider->minimum());

    if (slider->value()==oldMaxi)
        slider->setValue(slider->maximum());
}





/********************************
  *  Input settings
  *******************************/

/**
  Request the current config from the board
  */
void ConfigServoWidget::requestRCInputUpdate()
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(objManager->getObject(QString("ManualControlSettings")));
    Q_ASSERT(obj);
    obj->requestUpdate();

    // Now update all the slider values:
    QString fieldName = QString("ChannelMax");
    UAVObjectField *field = obj->getField(fieldName);
    m_config->ch0Max->setText(field->getValue(0).toString());
    m_config->ch0Slider->setMaximum(field->getValue(0).toInt());
    m_config->ch1Max->setText(field->getValue(1).toString());
    m_config->ch1Slider->setMaximum(field->getValue(1).toInt());
    m_config->ch2Max->setText(field->getValue(2).toString());
    m_config->ch2Slider->setMaximum(field->getValue(2).toInt());
    m_config->ch3Max->setText(field->getValue(3).toString());
    m_config->ch3Slider->setMaximum(field->getValue(3).toInt());
    m_config->ch4Max->setText(field->getValue(4).toString());
    m_config->ch4Slider->setMaximum(field->getValue(4).toInt());
    m_config->ch5Max->setText(field->getValue(5).toString());
    m_config->ch5Slider->setMaximum(field->getValue(5).toInt());
    m_config->ch6Max->setText(field->getValue(6).toString());
    m_config->ch6Slider->setMaximum(field->getValue(6).toInt());
    m_config->ch7Max->setText(field->getValue(7).toString());
    m_config->ch7Slider->setMaximum(field->getValue(7).toInt());

    fieldName = QString("ChannelMin");
    field = obj->getField(fieldName);
    m_config->ch0Min->setText(field->getValue(0).toString());
    m_config->ch0Slider->setMinimum(field->getValue(0).toInt());
    m_config->ch1Min->setText(field->getValue(1).toString());
    m_config->ch1Slider->setMinimum(field->getValue(1).toInt());
    m_config->ch2Min->setText(field->getValue(2).toString());
    m_config->ch2Slider->setMinimum(field->getValue(2).toInt());
    m_config->ch3Min->setText(field->getValue(3).toString());
    m_config->ch3Slider->setMinimum(field->getValue(3).toInt());
    m_config->ch4Min->setText(field->getValue(4).toString());
    m_config->ch4Slider->setMinimum(field->getValue(4).toInt());
    m_config->ch5Min->setText(field->getValue(5).toString());
    m_config->ch5Slider->setMinimum(field->getValue(5).toInt());
    m_config->ch6Min->setText(field->getValue(6).toString());
    m_config->ch6Slider->setMinimum(field->getValue(6).toInt());
    m_config->ch7Min->setText(field->getValue(7).toString());
    m_config->ch7Slider->setMinimum(field->getValue(7).toInt());

    fieldName = QString("ChannelNeutral");
    field = obj->getField(fieldName);
    m_config->ch0Slider->setValue(field->getValue(0).toInt());
    m_config->ch1Slider->setValue(field->getValue(1).toInt());
    m_config->ch2Slider->setValue(field->getValue(2).toInt());
    m_config->ch3Slider->setValue(field->getValue(3).toInt());
    m_config->ch4Slider->setValue(field->getValue(4).toInt());
    m_config->ch5Slider->setValue(field->getValue(5).toInt());
    m_config->ch6Slider->setValue(field->getValue(6).toInt());
    m_config->ch7Slider->setValue(field->getValue(7).toInt());

    // Update receiver type
    fieldName = QString("InputMode");
    field = obj->getField(fieldName);
    m_config->receiverType->setCurrentIndex(m_config->receiverType->findText(field->getValue().toString()));

    // Reset all channel assignement dropdowns:
    m_config->ch0Assign->setCurrentIndex(0);
    m_config->ch1Assign->setCurrentIndex(0);
    m_config->ch2Assign->setCurrentIndex(0);
    m_config->ch3Assign->setCurrentIndex(0);
    m_config->ch4Assign->setCurrentIndex(0);
    m_config->ch5Assign->setCurrentIndex(0);
    m_config->ch6Assign->setCurrentIndex(0);
    m_config->ch7Assign->setCurrentIndex(0);

    // Update all channels assignements
    assignChannel(obj, field, QString("Roll"));
    assignChannel(obj, field, QString("Pitch"));
    assignChannel(obj, field, QString("Yaw"));
    assignChannel(obj, field, QString("Throttle"));
    assignChannel(obj, field, QString("FlightMode"));

}


/**
  * Sends the config to the board, without saving to the SD card
  */
void ConfigServoWidget::sendRCInputUpdate()
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(objManager->getObject(QString("ManualControlSettings")));
    Q_ASSERT(obj);
    // Now update all fields from the sliders:
    QString fieldName = QString("ChannelMax");
    UAVObjectField * field = obj->getField(fieldName);
    field->setValue(m_config->ch0Max->text().toInt(),0);
    field->setValue(m_config->ch1Max->text().toInt(),1);
    field->setValue(m_config->ch2Max->text().toInt(),2);
    field->setValue(m_config->ch3Max->text().toInt(),3);
    field->setValue(m_config->ch4Max->text().toInt(),4);
    field->setValue(m_config->ch5Max->text().toInt(),5);
    field->setValue(m_config->ch6Max->text().toInt(),6);
    field->setValue(m_config->ch7Max->text().toInt(),7);

    fieldName = QString("ChannelMin");
    field = obj->getField(fieldName);
    field->setValue(m_config->ch0Min->text().toInt(),0);
    field->setValue(m_config->ch1Min->text().toInt(),1);
    field->setValue(m_config->ch2Min->text().toInt(),2);
    field->setValue(m_config->ch3Min->text().toInt(),3);
    field->setValue(m_config->ch4Min->text().toInt(),4);
    field->setValue(m_config->ch5Min->text().toInt(),5);
    field->setValue(m_config->ch6Min->text().toInt(),6);
    field->setValue(m_config->ch7Min->text().toInt(),7);

    fieldName = QString("ChannelNeutral");
    field = obj->getField(fieldName);
    field->setValue(m_config->ch0Slider->value(),0);
    field->setValue(m_config->ch1Slider->value(),1);
    field->setValue(m_config->ch2Slider->value(),2);
    field->setValue(m_config->ch3Slider->value(),3);
    field->setValue(m_config->ch4Slider->value(),4);
    field->setValue(m_config->ch5Slider->value(),5);
    field->setValue(m_config->ch6Slider->value(),6);
    field->setValue(m_config->ch7Slider->value(),7);

    // Set RC Receiver type:
    fieldName = QString("InputMode");
    field = obj->getField(fieldName);
    field->setValue(m_config->receiverType->currentText());

    // Set Roll/Pitch/Yaw/Etc assignement:
    // Rule: if two channels have the same setting (which is wrong!) the higher channel
    // will get the setting.

    // First, reset all channel assignements:
    QStringList channelsList;
    channelsList << "Roll" << "Pitch" << "Yaw" << "Throttle" << "FlightMode";
    foreach (QString channel, channelsList) {
        field = obj->getField(channel);
        field->setValue(field->getOptions().last());
    }

    // Then assign according to current GUI state:
    if (m_config->ch0Assign->currentIndex() != 0) {
        field = obj->getField(m_config->ch0Assign->currentText());
        field->setValue(field->getOptions().at(0)); // -> This way we don't depend on channel naming convention
    }
    if (m_config->ch1Assign->currentIndex() != 0) {
        field = obj->getField(m_config->ch1Assign->currentText());
        field->setValue(field->getOptions().at(1));
    }
    if (m_config->ch2Assign->currentIndex() != 0) {
        field = obj->getField(m_config->ch2Assign->currentText());
        field->setValue(field->getOptions().at(2));
    }
    if (m_config->ch3Assign->currentIndex() != 0) {
        field = obj->getField(m_config->ch3Assign->currentText());
        field->setValue(field->getOptions().at(3));
    }
    if (m_config->ch4Assign->currentIndex() != 0) {
        field = obj->getField(m_config->ch4Assign->currentText());
        field->setValue(field->getOptions().at(4));
    }
    if (m_config->ch5Assign->currentIndex() != 0) {
        field = obj->getField(m_config->ch5Assign->currentText());
        field->setValue(field->getOptions().at(5));
    }
    if (m_config->ch6Assign->currentIndex() != 0) {
        field = obj->getField(m_config->ch6Assign->currentText());
        field->setValue(field->getOptions().at(6));
    }
    if (m_config->ch7Assign->currentIndex() != 0) {
        field = obj->getField(m_config->ch7Assign->currentText());
        field->setValue(field->getOptions().at(7));
    }

    // ... and send to the OP Board
    obj->updated();

}


/**
  Sends the config to the board and request saving into the SD card
  */
void ConfigServoWidget::saveRCInputObject()
{
    // Send update so that the latest value is saved
    sendRCInputUpdate();
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(objManager->getObject(QString("ManualControlSettings")));
    Q_ASSERT(obj);
    updateObjectPersistance(ObjectPersistence::OPERATION_SAVE, obj);
}


/**
  * Set the dropdown option for a channel Input assignement
  */
void ConfigServoWidget::assignChannel(UAVDataObject *obj, UAVObjectField *field, QString str)
{
    field = obj->getField(str);
    QStringList options = field->getOptions();
    switch (options.indexOf(field->getValue().toString())) {
    case 0:
        m_config->ch0Assign->setCurrentIndex(m_config->ch0Assign->findText(str));
        break;
    case 1:
        m_config->ch1Assign->setCurrentIndex(m_config->ch0Assign->findText(str));
        break;
    case 2:
        m_config->ch2Assign->setCurrentIndex(m_config->ch0Assign->findText(str));
        break;
    case 3:
        m_config->ch3Assign->setCurrentIndex(m_config->ch0Assign->findText(str));
        break;
    case 4:
        m_config->ch4Assign->setCurrentIndex(m_config->ch0Assign->findText(str));
        break;
    case 5:
        m_config->ch5Assign->setCurrentIndex(m_config->ch0Assign->findText(str));
        break;
    case 6:
        m_config->ch6Assign->setCurrentIndex(m_config->ch0Assign->findText(str));
        break;
    case 7:
        m_config->ch7Assign->setCurrentIndex(m_config->ch0Assign->findText(str));
        break;
    }
}

/**
  * Set the dropdown option for a channel output assignement
  */
void ConfigServoWidget::assignOutputChannel(UAVDataObject *obj, UAVObjectField *field, QString str)
{
    field = obj->getField(str);
    QStringList options = field->getOptions();
    switch (options.indexOf(field->getValue().toString())) {
    case 0:
        m_config->ch0Output->setCurrentIndex(m_config->ch0Output->findText(str));
        break;
    case 1:
        m_config->ch1Output->setCurrentIndex(m_config->ch1Output->findText(str));
        break;
    case 2:
        m_config->ch2Output->setCurrentIndex(m_config->ch2Output->findText(str));
        break;
    case 3:
        m_config->ch3Output->setCurrentIndex(m_config->ch3Output->findText(str));
        break;
    case 4:
        m_config->ch4Output->setCurrentIndex(m_config->ch4Output->findText(str));
        break;
    case 5:
        m_config->ch5Output->setCurrentIndex(m_config->ch5Output->findText(str));
        break;
    case 6:
        m_config->ch6Output->setCurrentIndex(m_config->ch6Output->findText(str));
        break;
    case 7:
        m_config->ch7Output->setCurrentIndex(m_config->ch7Output->findText(str));
        break;
    }
}



/**
  * Updates the slider positions and min/max values
  *
  */
void ConfigServoWidget::updateChannels(UAVObject* controlCommand)
{

    QString fieldName = QString("Connected");
    UAVObjectField *field = controlCommand->getField(fieldName);
    if (field->getValue().toBool()) {
        m_config->RCInputConnected->setText("RC Receiver Connected");
    } else {
        m_config->RCInputConnected->setText("RC Receiver Not Connected");
    }
    if (m_config->doRCInputCalibration->isChecked()) {
        if (firstUpdate) {
            // Increase the data rate from the board so that the sliders
            // move faster
            UAVObject::Metadata mdata = controlCommand->getMetadata();
            mdata.flightTelemetryUpdateMode = UAVObject::UPDATEMODE_PERIODIC;
            mccDataRate = mdata.flightTelemetryUpdatePeriod;
            mdata.flightTelemetryUpdatePeriod = 150;
            controlCommand->setMetadata(mdata);
        }
        fieldName = QString("Channel");
        field =  controlCommand->getField(fieldName);
        // Hey: if you find a nicer way of doing this, be my guest!
        this->updateChannelSlider(&*m_config->ch0Slider,
                            &*m_config->ch0Min,
                            &*m_config->ch0Max,
                            &*m_config->ch0Rev,field->getValue(0).toInt());
        this->updateChannelSlider(&*m_config->ch1Slider,
                            &*m_config->ch1Min,
                            &*m_config->ch1Max,
                            &*m_config->ch1Rev,field->getValue(1).toInt());
        this->updateChannelSlider(&*m_config->ch2Slider,
                            &*m_config->ch2Min,
                            &*m_config->ch2Max,
                            &*m_config->ch2Rev,field->getValue(2).toInt());
        this->updateChannelSlider(&*m_config->ch3Slider,
                            &*m_config->ch3Min,
                            &*m_config->ch3Max,
                            &*m_config->ch3Rev,field->getValue(3).toInt());
        this->updateChannelSlider(&*m_config->ch4Slider,
                            &*m_config->ch4Min,
                            &*m_config->ch4Max,
                            &*m_config->ch4Rev,field->getValue(4).toInt());
        this->updateChannelSlider(&*m_config->ch5Slider,
                            &*m_config->ch5Min,
                            &*m_config->ch5Max,
                            &*m_config->ch5Rev,field->getValue(5).toInt());
        this->updateChannelSlider(&*m_config->ch6Slider,
                            &*m_config->ch6Min,
                            &*m_config->ch6Max,
                            &*m_config->ch6Rev,field->getValue(6).toInt());
        this->updateChannelSlider(&*m_config->ch7Slider,
                            &*m_config->ch7Min,
                            &*m_config->ch7Max,
                            &*m_config->ch7Rev,field->getValue(7).toInt());
        firstUpdate = false;
    } else  {
        if (!firstUpdate) {
            // Restore original data rate from the board:
            UAVObject::Metadata mdata = controlCommand->getMetadata();
            mdata.flightTelemetryUpdateMode = UAVObject::UPDATEMODE_PERIODIC;
            mdata.flightTelemetryUpdatePeriod = mccDataRate;
            controlCommand->setMetadata(mdata);
        }
        firstUpdate = true;
    }
}


void ConfigServoWidget::updateChannelSlider(QSlider* slider, QLabel* min, QLabel* max, QCheckBox* rev, int value) {

    if (firstUpdate) {
        // Reset all the min/max values of the sliders since we are
        // starting the calibration.
        slider->setMaximum(value);
        slider->setMinimum(value);
        slider->setValue(value);
        max->setText(QString::number(value));
        min->setText(QString::number(value));
        return;
    }

    if (value != 0) { // Avoids glitches...
        if (value > slider->maximum()) {
            slider->setMaximum(value);
            max->setText(QString::number(value));
        }
        if (value < slider->minimum()) {
            slider->setMinimum(value);
            min->setText(QString::number(value));
        }
        slider->setValue(value);
    }
}

