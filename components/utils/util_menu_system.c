//*****************************************************************************
//! @file       util_menu_system.c
//! @brief      Menu implementation for a generic menu system with sub menus.
//!             The menu system itself is hardware independent but a separate
//!             menu driver will be needed to display the menu on a given LCD
//!             display.
//!
//! Revised     $Date: 2014-01-23 09:55:53 +0100 (to, 23 jan 2014) $
//! Revision    $Revision: 11971 $
//
//  Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/
//
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions
//  are met:
//
//    Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//
//    Neither the name of Texas Instruments Incorporated nor the names of
//    its contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
//  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
//  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//****************************************************************************/


/**************************************************************************//**
* @addtogroup   menu_system_api
* @{
******************************************************************************/


/******************************************************************************
 * INCLUDES
 */
#include "util_menu_system.h"
#include "util_menu_driver.h"
#include "hal_types.h"


/**************************************************************************//**
* @brief    Takes in a menu and moves the selection one item/screen up.
*
* @param    pMenu       Menu to move selection on (the current menu)
*
* @return   Returns 1 if position changed, 0 if not (i.e. already in the top
*           of the menu)
******************************************************************************/
uint8 menuUp(menu_t *pMenu)
{
  int8 attemptedItemScreen, currentItemScreen, currentScreen;

  int8 attemptedItem = pMenu->nCurrentItem - 1;
  uint8 itemFound = 0;

  // Try to locate new item to choose
  while(attemptedItem>=0 && !itemFound)
  {
    if(pMenu->pItems[attemptedItem].flags & M_DISABLED)
    {
      // item disabled, try a new one
      attemptedItem--;
    }
    else
    {
      // item not disabled, try this one
      itemFound = 1;
    }
  }

  // If new item found on this screen and old item on this screen: change item
  // If new item found on previous screen and old on this: change screen and item
  // Else if this is not first screen: change screen only

  attemptedItemScreen = menuGetScreen(pMenu,attemptedItem);
  currentItemScreen = menuGetScreen(pMenu,pMenu->nCurrentItem);
  currentScreen = pMenu->nScreen;

  if(itemFound && attemptedItemScreen==currentScreen && currentItemScreen==currentScreen)
  {
    pMenu->nCurrentItem = attemptedItem;
    return 1;
  }
  else if(itemFound && attemptedItemScreen==currentScreen-1 && currentItemScreen>=currentScreen)
  {
    pMenu->nCurrentItem = attemptedItem;
    pMenu->nScreen--;
    return 1;
  }
  else if(pMenu->nScreen>0)
  {
    pMenu->nScreen--;
    return 1;
  }
  else
  {
    return 0;
  }

}

/**************************************************************************//**
* @brief    Takes in a menu and moves the selection one item down
*
* @param    pMenu       Menu to move selection on (the current menu)
*
* @return   Returns 1 if position changed, 0 if not (i.e. already at the
*           bottom of the menu)
******************************************************************************/
uint8 menuDown(menu_t *pMenu)
{
  // Try to locate a new item to choose

  uint8 lastScreen, attemptedItemScreen, currentItemScreen, currentScreen;

  uint8 attemptedItem = pMenu->nCurrentItem + 1;
  uint8 itemFound = 0;
  while(attemptedItem<pMenu->nMenuItems && !itemFound)
  {
    if(pMenu->pItems[attemptedItem].flags & M_DISABLED)
    {
      // item disabled, try a new one
      attemptedItem++;
    }
    else
    {
      // item not disabled, try this one
      itemFound = 1;
    }
  }

  // If new item found on this screen and old item on this screen: change item
  // If new item found on next screen and old on this: change screen and item
  // Else if this is not last screen: change screen only
  attemptedItemScreen = menuGetScreen(pMenu,attemptedItem);
  currentItemScreen = menuGetScreen(pMenu,pMenu->nCurrentItem);
  currentScreen = pMenu->nScreen;
  if(pMenu->nMenuItems>0)
  {
  	lastScreen = menuGetScreen(pMenu,pMenu->nMenuItems-1);
  }
  else
  {
    lastScreen = 0;
  }

  if(itemFound && attemptedItemScreen==currentScreen && currentItemScreen==currentScreen)
  {
    pMenu->nCurrentItem = attemptedItem;
    return 1;
  }
  else if(itemFound && attemptedItemScreen==currentScreen+1 && currentItemScreen<=currentScreen)
  {
    pMenu->nCurrentItem = attemptedItem;
    pMenu->nScreen++;
    return 1;
  }
  else if(pMenu->nScreen<lastScreen)
  {
    pMenu->nScreen++;
    return 1;
  }
  else
  {
    return 0;
  }
}

/**************************************************************************//**
* @brief    Takes in a menu and moves the selection to the first item
*
* @param    pMenu       Menu to move selection on (the current menu).
*
* @return   None
******************************************************************************/
void menuPositionTop(menu_t *pMenu){
  while(menuUp(pMenu));
}

/**************************************************************************//**
* @brief    Enters the selected menuItem on the input argument. This means:
*           If an application is provided by the menuItem (i.e. it's not NULL)
*           then run the application with the arguments provided by the
*           menuItem in pAppArgs. Afterwards, return submenu specified
*           in pSubMenu (if not NULL) to update pCurrentMenu with. Also, if
*           the "select option menu" is used, that is, nSelectedItem is in use
*           and not (-1), current item get's selected.
*
* @param    pMenu       Pointer to the menu the user's currently in
*
* @return   Pointer to the menu the user should be in after entering.
******************************************************************************/
menu_t* menuEnter(menu_t *pMenu)
{
  int8 nCurrentItem = pMenu->nCurrentItem;
  int8 nCurrentItemScreen = menuGetScreen(pMenu,nCurrentItem);

  if(nCurrentItemScreen==pMenu->nScreen)
  {
    pAppFunction pApp = pMenu->pItems[nCurrentItem].pApplication;
    void *pAppArgs = pMenu->pItems[nCurrentItem].pAppArgs;
    menu_t *pSub = pMenu->pItems[nCurrentItem].pSubMenu;

    if(pMenu->nSelectedItem > -1)
    {
      pMenu->nSelectedItem=nCurrentItem;
    }
    if(pApp)
    {
    	// save the return value of pApp, if 0 (success) move on, if 1 return pMenu
    	if((*pApp)(&pAppArgs)) return pMenu;
    }

    if(pSub)
    {
    	pSub->pParentMenu = pMenu;
        return pSub;
    }
    else
    {
      return pMenu;
    }
  }
  else
  {
  	// if not on current item's screen, you're on a screen with no
  	// enabled items. Then do nothing.
  	//
    return pMenu;
  }
}

/**************************************************************************//**
* @brief    Steps out of the current menu and into the parent one if applicable.
*
* @param    pMenu       Pointer to the menu the user's currently in
*
* @return   Pointer to the (parent) menu the user should be in after entering.
*           Returns the input menu if no parent menu is presented.
******************************************************************************/
menu_t* menuBack(menu_t *pMenu)
{
  if(pMenu->pParentMenu)
  {
    // Reset position before leaving menu.
    // If this is an option-menu it should be reset to selected option.
    // else, go to the top.
    if(pMenu->nSelectedItem==-1)
    {
      menuPositionTop(pMenu); // resets screen and item
    }
    else if(pMenu->nSelectedItem == -2)
    {
        // do nothing
    }
    else
    {
      pMenu->nCurrentItem = pMenu->nSelectedItem;
      pMenu->nScreen = menuGetScreen(pMenu,pMenu->nSelectedItem);
    }
    return pMenu->pParentMenu;
  }
  else
  {
    return pMenu;
  }
}

/**************************************************************************//**
* @brief    Steps out of the current menu and into the uppermost (top) menu.
*
* @param    pMenu       Pointer to the menu the user's currently in
*
* @return   Pointer to the uppermost (top) menu.
******************************************************************************/
menu_t* menuTop(const menu_t *pMenu)
{
  menu_t *pTopMenu;
  pTopMenu = (menu_t*)pMenu;
  while(pTopMenu->pParentMenu)
  {
    pTopMenu = pTopMenu->pParentMenu;
  }
  return pTopMenu;
}


/**************************************************************************//**
* Close the Doxygen group.
* @}
******************************************************************************/
