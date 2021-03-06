<?php
namespace LAM\TOOLS\TESTS;
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2009 - 2016  Roland Gruber

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

/**
* This file includes the test tool specification.
*
* @author Roland Gruber
* @package tools
*/

/**
 * Tests page
 *
 * @package tools
 */
class toolTests implements \LAMTool {

	/**
	 * Returns the name of the tool.
	 *
	 * @return string name
	 */
	 function getName() {
	 	return _("Tests");
	 }

	/**
	 * returns a description text for the tool.
	 *
	 * @return string description
	 */
	function getDescription() {
		return _("Here you can test if certain LAM features work on your installation.");
	}

	/**
	 * Returns a link to the tool page (relative to templates/).
	 *
	 * @return string link
	 */
	function getLink() {
		return "tests/index.php";
	}

	/**
	 * Returns if the tool requires write access to LDAP.
	 *
	 * @return boolean true if write access is needed
	 */
	function getRequiresWriteAccess() {
		return true;
	}

	/**
	 * Returns if the tool requires password change rights.
	 *
	 * @return boolean true if password change rights are needed
	 */
	function getRequiresPasswordChangeRights() {
		return true;
	}

	/**
	 * Returns the link to the tool image (relative to graphics/)
	 *
	 * @return string image URL
	 */
	function getImageLink() {
		return 'tests.png';
	}

	/**
	 * Returns the preferred position of this tool on the tools page.
	 * The position may be between 0 and 1000. 0 is the top position.
	 *
	 * @return int preferred position
	 */
	function getPosition() {
		return 1000;
	}

	/**
	 * Returns a list of sub tools or an empty array.
	 *
	 * @return array list of subtools (LAMTool)
	 */
	function getSubTools() {
		$return = array();
		$lamdaemonTest = new \LAMSubTool();
		$lamdaemonTest->name = _("Lamdaemon test");
		$lamdaemonTest->link = 'tests/lamdaemonTest.php';
		$lamdaemonTest->description = _("Check if quotas and homedirectories can be managed.");
		$lamdaemonTest->image = 'lamdaemonSmall.png';
		$return[] = $lamdaemonTest;
		$schemaTest = new \LAMSubTool();
		$schemaTest->name = _("Schema test");
		$schemaTest->link = 'tests/schemaTest.php';
		$schemaTest->description = _("Check if the LDAP schema fits the requirements of the selected account modules.");
		$schemaTest->image = 'schemaTest.png';
		$return[] = $schemaTest;
		return $return;
	}

	/**
	 * Returns if the tool is visible in the menu.
	 *
	 * @return boolean visible
	 */
	function isVisible() {
		$typeManager = new \LAM\TYPES\TypeManager();
		$types = $typeManager->getConfiguredTypes();
		return (sizeof($types) > 0);
	}

	/**
	 * Returns if a tool may be hidden by configuration in the LAM server profile.
	 *
	 * @return boolean hideable
	 */
	function isHideable() {
		return true;
	}

}

?>