/****************************************************************************
 * Copyright (C) 2015 Dimok
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#include <fs/DirList.h>
#include <fs/FSUtils.h>
#include <utils/utils.h>

#include "HomebrewLaunchWindow.h"
#include "HomebrewLoader.h"
#include "common/common.h"
#include "custom/utils/HomebrewXML.h"
#include "Application.h"

HomebrewLaunchWindow::HomebrewLaunchWindow(const std::string & launchPath, GuiImageData * iconImgData)
    : GuiFrame(0, 0)
    , buttonClickSound(Resources::GetSound("button_click.mp3"))
    , backgroundImgData(Resources::GetImageData("launchMenuBox.png"))
    , backgroundImg(backgroundImgData)
    , buttonImgData(Resources::GetImageData("button.png"))
    , buttonSelectedImgData(Resources::GetImageData("buttonSelected.png"))
    , iconImage(iconImgData)
    , titleText((char*)NULL, 42, glm::vec4(1.0f))
    , versionText("バージョン:", 32, glm::vec4(1.0f))
    , versionValueText((char*)NULL, 32, glm::vec4(1.0f))
    , authorText("著者名:", 32, glm::vec4(1.0f))
    , authorValueText((char*)NULL, 32, glm::vec4(1.0f))
    , descriptionText((char*)NULL, 28, glm::vec4(1.0f))
    , loadBtnLabel("ロード", 32, glm::vec4(1.0f))
    , loadImg(buttonImgData)
    , loadSelectedImg(buttonSelectedImgData)
    , loadBtn(loadImg.getWidth(), loadImg.getHeight())
    , backBtnLabel("戻る", 32, glm::vec4(1.0f))
    , backImg(buttonImgData)
    , backSelectedImg(buttonSelectedImgData)
    , backBtn(backImg.getWidth(), backImg.getHeight())
    , touchTrigger(GuiTrigger::CHANNEL_1, GuiTrigger::VPAD_TOUCH)
    , wpadTouchTrigger(GuiTrigger::CHANNEL_2 | GuiTrigger::CHANNEL_3 | GuiTrigger::CHANNEL_4 | GuiTrigger::CHANNEL_5, GuiTrigger::BUTTON_A)
    , buttonRightTrigger(GuiTrigger::CHANNEL_ALL, GuiTrigger::BUTTON_RIGHT, true)
    , buttonLeftTrigger(GuiTrigger::CHANNEL_ALL, GuiTrigger::BUTTON_LEFT, true)
    , buttonATrigger(GuiTrigger::CHANNEL_ALL, GuiTrigger::BUTTON_A, true)
    , buttonBTrigger(GuiTrigger::CHANNEL_ALL, GuiTrigger::BUTTON_B, true)
    , dpadButtons(0, 0)
    , buttons (0, 0)
    , homebrewLaunchPath(launchPath)
{
    width = backgroundImg.getWidth();
    height = backgroundImg.getHeight();
    append(&backgroundImg);

    std::string homebrewPath = launchPath;
    size_t slashPos = homebrewPath.rfind('/');
    if(slashPos != std::string::npos)
        homebrewPath.erase(slashPos);

    HomebrewXML metaXml;
    bool xmlReadSuccess = metaXml.LoadHomebrewXMLData((homebrewPath + "/meta.xml").c_str());

    int xOffset = 500;
    int yOffset = height * 0.5f - 75.0f;

    const char *cpName = xmlReadSuccess ? metaXml.GetName() : launchPath.c_str();
    if(strncmp(cpName, "sd:/wiiu/apps/", strlen("sd:/wiiu/apps/")) == 0)
       cpName += strlen("sd:/wiiu/apps/");

    titleText.setText(cpName);
    titleText.setAlignment(ALIGN_CENTER | ALIGN_MIDDLE);
    titleText.setPosition(0, yOffset);
    titleText.setMaxWidth(width - 100, GuiText::DOTTED);
    append(&titleText);

    float scaleFactor = 1.0f;
    iconImage.setAlignment(ALIGN_LEFT | ALIGN_MIDDLE);
    iconImage.setPosition(100, yOffset - 30 - iconImage.getHeight() * 0.5f * scaleFactor);
    iconImage.setScale(scaleFactor);
    append(&iconImage);

    yOffset -= 50;

    versionText.setAlignment(ALIGN_LEFT | ALIGN_MIDDLE);
    versionText.setPosition(width - xOffset, yOffset);
    append(&versionText);

    versionValueText.setTextf("%s", xmlReadSuccess ? metaXml.GetVersion() : launchPath.c_str());
    versionValueText.setAlignment(ALIGN_LEFT | ALIGN_MIDDLE);
    versionValueText.setPosition(width - xOffset + 100, yOffset);
    versionValueText.setMaxWidth(xOffset - 150, GuiText::DOTTED);
    append(&versionValueText);
    yOffset -= 30;

    authorText.setAlignment(ALIGN_LEFT | ALIGN_MIDDLE);
    authorText.setPosition(width - xOffset, yOffset);
    append(&authorText);

    authorValueText.setTextf("%s", metaXml.GetCoder());
    authorValueText.setAlignment(ALIGN_LEFT | ALIGN_MIDDLE);
    authorValueText.setPosition(width - xOffset + 100, yOffset);
    authorValueText.setMaxWidth(xOffset - 150, GuiText::DOTTED);
    append(&authorValueText);
    yOffset -= 50;

    descriptionText.setText(metaXml.GetLongDescription());
    descriptionText.setAlignment(ALIGN_LEFT | ALIGN_TOP);
    descriptionText.setPosition(100, -250);
    descriptionText.setMaxWidth(width - 200, GuiText::WRAP);
    append(&descriptionText);

    scaleFactor = 1.0f;
    loadImg.setScale(scaleFactor);
    loadBtn.setSize(scaleFactor * loadImg.getWidth(), scaleFactor * loadImg.getHeight());
    loadBtn.setImage(&loadImg);
    loadBtn.setLabel(&loadBtnLabel);
    loadBtn.setAlignment(ALIGN_CENTER | ALIGN_MIDDLE);
    loadBtn.setPosition(-200, -310);
    loadBtn.setTrigger(&touchTrigger);
    loadBtn.setTrigger(&wpadTouchTrigger);
    loadBtn.setEffectGrow();
    loadBtn.setSoundClick(buttonClickSound);
    loadBtn.setSelectable(true);
    loadBtn.setImageOver(&loadSelectedImg);
    loadBtn.setdrawOverOnlyWhenSelected(true);
    backBtn.setEffectGrow();
    loadBtn.clicked.connect(this, &HomebrewLaunchWindow::OnLoadButtonClick);
    append(&loadBtn);

    backImg.setScale(scaleFactor);
    backBtn.setSize(scaleFactor * backImg.getWidth(), scaleFactor * backImg.getHeight());
    backBtn.setImage(&backImg);
    backBtn.setLabel(&backBtnLabel);
    backBtn.setAlignment(ALIGN_CENTER | ALIGN_MIDDLE);
    backBtn.setPosition(200, -310);
    backBtn.setTrigger(&touchTrigger);
    backBtn.setTrigger(&wpadTouchTrigger);
    backBtn.setEffectGrow();
    backBtn.setSoundClick(buttonClickSound);
    backBtn.setSelectable(true);
    backBtn.setImageOver(&backSelectedImg);
    backBtn.setdrawOverOnlyWhenSelected(true);
    backBtn.setEffectGrow();
    backBtn.clicked.connect(this, &HomebrewLaunchWindow::OnBackButtonClick);
    append(&backBtn);

    dpadButtons.setTrigger(&buttonRightTrigger);
    dpadButtons.setTrigger(&buttonLeftTrigger);
    dpadButtons.clicked.connect(this, &HomebrewLaunchWindow::OnDpadClick);
    append(&dpadButtons);

    buttons.setTrigger(&buttonATrigger);
    buttons.setTrigger(&buttonBTrigger);
    buttons.clicked.connect(this, &HomebrewLaunchWindow::OnButtonClick);
    append(&buttons);
}

HomebrewLaunchWindow::~HomebrewLaunchWindow()
{
    Resources::RemoveSound(buttonClickSound);
    Resources::RemoveImageData(backgroundImgData);
    Resources::RemoveImageData(buttonImgData);
    Resources::RemoveImageData(buttonSelectedImgData);
}

void HomebrewLaunchWindow::OnOpenEffectFinish(GuiElement *element)
{
    //! once the menu is open reset its state and allow it to be "clicked/hold"
    element->effectFinished.disconnect(this);
    element->clearState(GuiElement::STATE_DISABLED);
}

void HomebrewLaunchWindow::OnCloseEffectFinish(GuiElement *element)
{
    //! remove element from draw list and push to delete queue
    remove(element);
    AsyncDeleter::pushForDelete(element);

    backBtn.clearState(GuiElement::STATE_DISABLED);
    loadBtn.clearState(GuiElement::STATE_DISABLED);
}

void HomebrewLaunchWindow::OnFileLoadFinish(GuiElement *element, const std::string & filepath, int result)
{
    element->setState(GuiElement::STATE_DISABLED);
    element->setEffect(EFFECT_FADE, -10, 0);
    element->effectFinished.connect(this, &HomebrewLaunchWindow::OnCloseEffectFinish);

    if(result > 0)
    {
        Application::instance()->quit(EXIT_SUCCESS);
    }
}

void HomebrewLaunchWindow::OnLoadButtonClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
{
    // If there is no pointer dont click
    if (trigger == &wpadTouchTrigger && !((ControllerBase*) controller)->showPointer)
    {
        return;
    }

    backBtn.setState(GuiElement::STATE_DISABLED);
    loadBtn.setState(GuiElement::STATE_DISABLED);

    HomebrewLoader * loader = HomebrewLoader::loadToMemoryAsync(homebrewLaunchPath);
    loader->setEffect(EFFECT_FADE, 15, 255);
    loader->effectFinished.connect(this, &HomebrewLaunchWindow::OnOpenEffectFinish);
    loader->asyncLoadFinished.connect(this, &HomebrewLaunchWindow::OnFileLoadFinish);
    append(loader);
}

int HomebrewLaunchWindow::findSelectedButton()
{
    if (loadBtn.isStateSet(STATE_SELECTED))
        return 0;
    if (backBtn.isStateSet(STATE_SELECTED))
        return 1;

    return -1;
}

void HomebrewLaunchWindow::OnDpadClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
{
    selected(controller);

    int index = findSelectedButton();

    if (trigger == &buttonLeftTrigger)
    {
        if (index < 0)
        {
            loadBtn.setState(STATE_SELECTED);
        }
        else if (index == 1)
        {
            backBtn.clearState(STATE_SELECTED);
            loadBtn.setState(STATE_SELECTED);
        }
    }
    
    if (trigger == &buttonRightTrigger)
    {
        if (index < 0)
        {
            loadBtn.setState(STATE_SELECTED);
        }
        else if (index == 0)
        {
            backBtn.setState(STATE_SELECTED);
            loadBtn.clearState(STATE_SELECTED);
        }
    }
}

void HomebrewLaunchWindow::OnButtonClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
{
    if (trigger == &buttonBTrigger)
        backButtonClicked(this);

    if (trigger == &buttonATrigger)
    {
        // If the controller has a pointer on screen
        if(((ControllerBase*) controller)->showPointer)
            return;

        int index = findSelectedButton();
        if (index == 0)
        {
            loadBtn.clicked(&loadBtn, controller, trigger);
        }
        else if (index == 1)
        {
            backButtonClicked(this);
        }
        
    }
}
