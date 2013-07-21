<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2006  Tilo Lutz
                2007 - 2012  Roland Gruber

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
* Manages quotas for users and groups.
*
* @package modules
*
* @author Tilo Lutz
* @author Roland Gruber
* @author Michael Duergner
*/

/**
* Manages quotas for users and groups.
*
* @package modules
*/
class quota extends baseModule {
	
	/** delimiter for lamdaemon commands */
	private static $SPLIT_DELIMITER = "###x##y##x###";
	/** prefix for lamdaemon results */
	private static $QUOTA_PREFIX = 'QUOTA_ENTRY ';

	/** this functin fills the error message array with messages
	**/
	function load_Messages() {
		// error messages for input checks
		$this->messages['softblock'][0] = array('ERROR', _('Block soft quota'), _('Block soft quota contains invalid characters. Only natural numbers are allowed.'));
		$this->messages['softblock'][1] = array('ERROR', _('Account %s:') . ' %s', _('Block soft quota contains invalid characters. Only natural numbers are allowed.'));
		$this->messages['hardblock'][0] = array('ERROR', _('Block hard quota'), _('Block hard quota contains invalid characters. Only natural numbers are allowed.'));
		$this->messages['hardblock'][1] = array('ERROR', _('Account %s:') . ' %s', _('Block hard quota contains invalid characters. Only natural numbers are allowed.'));
		$this->messages['softinode'][0] = array('ERROR', _('Inode soft quota'), _('Inode soft quota contains invalid characters. Only natural numbers are allowed.'));
		$this->messages['softinode'][1] = array('ERROR', _('Account %s:') . ' %s', _('Inode soft quota contains invalid characters. Only natural numbers are allowed.'));
		$this->messages['hardinode'][0] = array('ERROR', _('Inode hard quota'), _('Inode hard quota contains invalid characters. Only natural numbers are allowed.'));
		$this->messages['hardinode'][1] = array('ERROR', _('Account %s:') . ' %s', _('Inode hard quota contains invalid characters. Only natural numbers are allowed.'));
		$this->messages['block_cmp'][0] = array('ERROR', _('Block quota'), _('Block soft quota must be smaller than block hard quota.'));
		$this->messages['block_cmp'][1] = array('ERROR', _('Account %s:') . ' %s', _('Block soft quota must be smaller than block hard quota.'));
		$this->messages['inode_cmp'][0] = array('ERROR', _('Inode quota'), _('Inode soft quota must be smaller than inode hard quota.'));
		$this->messages['inode_cmp'][1] = array('ERROR', _('Account %s:') . ' %s', _('Inode soft quota must be smaller than inode hard quota.'));
		$this->messages['upload'][0] = array('ERROR', _('Account %s:') . ' %s', _('Quota has wrong format!'));
	}

	/**
	* Returns meta data that is interpreted by parent class
	*
	* @return array array with meta data
	* 
	* @see baseModule::get_metaData()
	*/
	function get_metaData() {
		$return = array();
		// icon
		$return['icon'] = 'hard-driveBig.png';
		// manages user and group accounts
		$return["account_types"] = array("user", "group");
		// alias name
		$return["alias"] = _('Quota');
		if ($this->get_scope() == 'group') {
			// module dependencies
			$return['dependencies'] = array('depends' => array(array('posixGroup', 'rfc2307bisPosixGroup')), 'conflicts' => array());
		}
		if ($this->get_scope() == 'user') {
			// module dependencies
			$return['dependencies'] = array('depends' => array('posixAccount'), 'conflicts' => array());
		}
		// managed attributes
		$return['attributes'] = array('uid', 'cn');
		// available PDF fields
		$return['PDF_fields'] =  array(
			'quotas' => _('Quota')
		);
		// help entries
		$return['help'] = array(
			"Mountpoint" => array(
				"Headline" => _("Mountpoint"),
				"Text" => _("Mountpoint of device with enabled quotas.")
			),
			"UsedBlocks" => array(
				"Headline" => _("Used blocks"),
				"Text" => _("Used blocks. 1000 blocks are usually 1MB")
			),
			"SoftBlockLimit" => array(
				"Headline" => _("Soft block limit"),
				"Text" => _("Soft block limit."), "SeeAlso" => array(
					'link' => 'http://www.ibiblio.org/pub/Linux/docs/HOWTO/other-formats/html_single/Quota.html#ss4.4',
					'text' => 'Quota How-To')
			),
			"HardBlockLimit" => array(
				"Headline" => _("Hard block limit"),
				"Text" => _("Hard block limit"), "SeeAlso" => array(
					'link' => 'http://www.ibiblio.org/pub/Linux/docs/HOWTO/other-formats/html_single/Quota.html#ss4.5',
					'text' => 'Quota How-To')
			),
			"GraceBlockPeriod" => array(
				"Headline" => _("Grace block period"),
				"Text" => _("Grace block period. Most filesystems use a fixed maximum value of 7 days."), "SeeAlso" => array(
					'link' => 'http://www.ibiblio.org/pub/Linux/docs/HOWTO/other-formats/html_single/Quota.html#ss4.6',
					'text' => 'Quota How-To')
			),
			"UsedInodes" => array(
				"Headline" => _("Used inodes"),
				"Text" => _("Used inodes (files)").'.'
			),
			"SoftInodeLimit" => array(
				"Headline" => _("Soft inode limit"),
				"Text" => _("Soft inode (files) limit."), "SeeAlso" => array(
					'link' => 'http://www.ibiblio.org/pub/Linux/docs/HOWTO/other-formats/html_single/Quota.html#ss4.4',
					'text' => 'Quota How-To')
			),
			"HardInodeLimit" => array(
				"Headline" => _("Hard inode limit"),
				"Text" => _("Hard inode (files) limit"), "SeeAlso" => array(
					'link' => 'http://www.ibiblio.org/pub/Linux/docs/HOWTO/other-formats/html_single/Quota.html#ss4.5',
					'text' => 'Quota How-To')
			),
			"GraceInodePeriod" => array(
				"Headline" => _("Grace inode period"),
				"Text" => _("Grace inode (files) period. Most filesystems use a fixed maximum value of 7 days."), "SeeAlso" => array(
					'link' => 'http://www.ibiblio.org/pub/Linux/docs/HOWTO/other-formats/html_single/Quota.html#ss4.6',
					'text' => 'Quota How-To')
			),
			"upload" => array(
				"Headline" => _("Quota"),
				"Text" => _("Please enter the quota settings for this mount point. The syntax is: {soft block limit},{hard block limit},{soft inode limit},{hard inode limit}")
			)
		);
		return $return;
	}

	/** Saves the quota settings */
	private $quota;
	
	/**
	 * Initializes the quota values.
	 *
	 */
	function initQuotas() {
		if (isset($this->quota)) return;
		$userName = '+';
		if (($this->getAccountContainer() != null) && !$this->getAccountContainer()->isNewAccount) {
			if ($this->get_scope() == 'user') {
				if (!isset($this->attributes['uid'][0])) return;
				$userName = $this->attributes['uid'][0];
			}
			else if ($this->get_scope() == 'group') {
				if (!isset($this->attributes['cn'][0])) return;
				$userName = $this->attributes['cn'][0];
			}
		}
		// get list of lamdaemon servers
		$lamdaemonServers = explode(";", $_SESSION['config']->get_scriptServers());
		for ($s = 0; $s < sizeof($lamdaemonServers); $s++) {
			$temp = explode(":", $lamdaemonServers[$s]);
			$server = $temp[0];
			// get quotas
			$quotas = lamdaemon(implode(quota::$SPLIT_DELIMITER, array($userName, "quota", "get", $this->get_scope())), $server);
			if (sizeof($quotas) == 0) {
				continue;
			}
			$allQuotas = explode(":", $quotas[0]);
			array_pop($allQuotas); // remove empty element at the end
			for ($i = 0; $i < sizeof($allQuotas); $i++) {
				if (strpos($allQuotas[$i], quota::$QUOTA_PREFIX) !== 0) continue;
				$allQuotas[$i] = substr($allQuotas[$i], strlen(quota::$QUOTA_PREFIX));
				$singleQuota = explode(",", $allQuotas[$i]);
				$this->quota[$server][$i] = $singleQuota;
				if ($this->quota[$server][$i][4] < time()) $this->quota[$server][$i][4] = '';
				else $this->quota[$server][$i][4] = strval(intval(($this->quota[$server][$i][4] - time())/3600)) .' '. _('hours');
				if ($this->quota[$server][$i][8] < time()) $this->quota[$server][$i][8] = '';
				else $this->quota[$server][$i][8] = strval(intval(($this->quota[$server][$i][8] - time())/3600)) .' '. _('hours');
			}
		}
	}

	/**
	 * Gets the cn from the Unix group module.
	 *
	 * @return String cn attribute
	 */
	private function getCn() {
		$modules = array('posixGroup', 'groupOfNames', 'groupOfUniqueNames');
		for ($i = 0; $i < sizeof($modules); $i++) {
			if ($this->getAccountContainer()->getAccountModule($modules[$i]) != null) {
				$attrs = $this->getAccountContainer()->getAccountModule($modules[$i])->getAttributes();
				if (isset($attrs['cn'][0])) {
					return $attrs['cn'][0];
				}
			}
		}
		return null;
	}
	
	/**
	* This function is used to check if this module page can be displayed.
	* It returns false if a module depends on data from other modules which was not yet entered.
	*
	* @return boolean true, if page can be displayed
	*/
	function module_ready() {
		if ($this->get_scope()=='user') {
			$attrs = $this->getAccountContainer()->getAccountModule('posixAccount')->getAttributes();
			if ($attrs['uid'][0]=='') return false;
		}
		if ($this->get_scope()=='group') {
			if (($this->getCn() == null) || ($this->getCn() == '')) {
				return false;
			}
		}
		return true;
	}

	/**
	 * Quotas are set in postmodify.
	 * 
	 * @see baseModule::postModifyActions()
	 *
	 * @param boolean $newAccount is new account
	 * @param array $attributes LDAP attributes of this entry
	 * @return array array which contains status messages. Each entry is an array containing the status message parameters.
	 */
	public function postModifyActions($newAccount, $attributes) {
		$messages = array();
		if (!isset($this->quota) || !is_array($this->quota)) return $messages;
		// determine if this is a user or group account
		if ($this->get_scope()=='user') {
			$attrs = $this->getAccountContainer()->getAccountModule('posixAccount')->getAttributes();
			$id = $attrs['uid'][0];
		}
		if ($this->get_scope()=='group') {
			$id = $this->getCn();
		}
		// get list of lamdaemon servers
		$serverDescriptions = array();
		$lamdaemonServers = explode(";", $_SESSION['config']->get_scriptServers());
		for ($s = 0; $s < sizeof($lamdaemonServers); $s++) {
			$temp = explode(":", $lamdaemonServers[$s]);
			if (isset($temp[1])) {
				$serverDescriptions[$temp[0]] = $temp[1];
			}
		}
		$servers = array_keys($this->quota);
		for ($q = 0; $q < sizeof($servers); $q++) {
			$server = $servers[$q];
			$i=0;
			$quotastring = "";
			for ($i = 0; $i < sizeof($this->quota[$server]); $i++) {
				$quotastring = $quotastring . $this->quota[$server][$i][0] . ',' . $this->quota[$server][$i][2] . ',' . $this->quota[$server][$i][3]
					. ',' . $this->quota[$server][$i][6] . ',' . $this->quota[$server][$i][7] . ':';
			}
			lamdaemon(implode(quota::$SPLIT_DELIMITER, array($id, "quota", "set", $this->get_scope(), "$quotastring\n")), $server);
		}
		return $messages;
	}

	/**
	 * Allows the module to run commands before the LDAP entry is deleted.
	 * 
	 * @return array Array which contains status messages. Each entry is an array containing the status message parameters.
	 */
	function preDeleteActions() {
		$this->initQuotas();
		if (!isset($this->quota) || !is_array($this->quota)) return array();
		// determine if this is a user or group account
		if ($this->get_scope()=='user') {
			$attrs = $this->getAccountContainer()->getAccountModule('posixAccount')->getAttributes();
			$id = $attrs['uid'][0];
		}
		if ($this->get_scope()=='group') {
			$id = $this->getCn();
		}
		// get list of lamdaemon servers
		$serverDescriptions = array();
		$lamdaemonServers = explode(";", $_SESSION['config']->get_scriptServers());
		for ($s = 0; $s < sizeof($lamdaemonServers); $s++) {
			$temp = explode(":", $lamdaemonServers[$s]);
			if (isset($temp[1])) {
				$serverDescriptions[$temp[0]] = $temp[1];
			}
		}
		$servers = array_keys($this->quota);
		for ($q = 0; $q < sizeof($servers); $q++) {
			$server = $servers[$q];
			$i=0;
			$quotastring = "";
			while ($this->quota[$server][$i][0]) {
				$quotastring = $quotastring . $this->quota[$server][$i][0] . ',0,0,0,0:';
				$i++;
				}
			lamdaemon(implode(quota::$SPLIT_DELIMITER, array($id, "quota", "set", $this->get_scope(), "$quotastring\n")), $server);
		}
		return array();
	}

	/**
	* Processes user input of the primary module page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @return array list of info/error messages
	*/
	function process_attributes() {
		if (!isset($this->quota) || !is_array($this->quota)) return array();
		$errors = array();
		// get list of lamdaemon servers
		$serverDescriptions = array();
		$lamdaemonServers = explode(";", $_SESSION['config']->get_scriptServers());
		for ($s = 0; $s < sizeof($lamdaemonServers); $s++) {
			$temp = explode(":", $lamdaemonServers[$s]);
			if (isset($temp[1])) {
				$serverDescriptions[$temp[0]] = $temp[1];
			}
		}
		$servers = array_keys($this->quota);
		for ($q = 0; $q < sizeof($servers); $q++) {
			$server = $servers[$q];
			$id = $this->replaceSpecialChars($server);
			$i=0;
			// loop for every mointpoint with quotas
			while (isset($this->quota[$server][$i][0])) {
				$this->quota[$server][$i][2] = $_POST[$i . '_2_' . $id];
				$this->quota[$server][$i][3] = $_POST[$i . '_3_' . $id];
				$this->quota[$server][$i][6] = $_POST[$i . '_6_' . $id];
				$this->quota[$server][$i][7] = $_POST[$i . '_7_' . $id];
				// Check if values are OK and set automatic values. if not error-variable will be set
				if (!get_preg($this->quota[$server][$i][2], 'digit'))
					$errors[] = $this->messages['softblock'][0];
				if (!get_preg($this->quota[$server][$i][3], 'digit'))
					$errors[] = $this->messages['hardblock'][0];
				if (!get_preg($this->quota[$server][$i][6], 'digit'))
					$errors[] = $this->messages['softinode'][0];
				if (!get_preg($this->quota[$server][$i][7], 'digit'))
					$errors[] = $this->messages['hardinode'][0];
				if (intval($this->quota[$server][$i][2]) > intval($this->quota[$server][$i][3]))
					$errors[] = $this->messages['block_cmp'][0];
				if (intval($this->quota[$server][$i][6]) > intval($this->quota[$server][$i][7]))
					$errors[] = $this->messages['inode_cmp'][0];
				$i++;
			}
		}
		return $errors;
	}

	/**
	 * Returns the HTML meta data for the main account page.
	 * 
	 * @return htmlElement HTML meta data
	 */
	function display_html_attributes() {
		$return = new htmlTable();
		$this->initQuotas();
		if (!is_array($this->quota)) return $return;
		// get list of lamdaemon servers
		$serverDescriptions = array();
		$lamdaemonServers = explode(";", $_SESSION['config']->get_scriptServers());
		for ($s = 0; $s < sizeof($lamdaemonServers); $s++) {
			$temp = explode(":", $lamdaemonServers[$s]);
			if (isset($temp[1])) {
				$serverDescriptions[$temp[0]] = $temp[1];
			}
		}
		$servers = array_keys($this->quota);
		for ($q = 0; $q < sizeof($servers); $q++) {
			$server = $servers[$q];
			$id = $this->replaceSpecialChars($server);
			$title = $server;
			if (isset($serverDescriptions[$server])) {
				$title = $serverDescriptions[$server] . " (" . $server . ")";
			}
			$return->addElement(new htmlSubTitle($title), true);
			$return->addElement(new htmlOutputText('&nbsp;' . _('Mountpoint') . '&nbsp;', false));
			$return->addElement(new htmlOutputText('&nbsp;' . _('Used blocks') . '&nbsp;', false));
			$return->addElement(new htmlOutputText('&nbsp;' . _('Soft block limit') . '&nbsp;', false));
			$return->addElement(new htmlOutputText('&nbsp;' . _('Hard block limit') . '&nbsp;', false));
			$return->addElement(new htmlOutputText('&nbsp;' . _('Grace block period') . '&nbsp;', false));
			$return->addElement(new htmlOutputText('&nbsp;' . _('Used inodes') . '&nbsp;', false));
			$return->addElement(new htmlOutputText('&nbsp;' . _('Soft inode limit') . '&nbsp;', false));
			$return->addElement(new htmlOutputText('&nbsp;' . _('Hard inode limit') . '&nbsp;', false));
			$return->addElement(new htmlOutputText('&nbsp;' . _('Grace inode period') . '&nbsp;', false));
			$return->addNewLine();
			$return->addElement(new htmlHelpLink('Mountpoint'));
			$return->addElement(new htmlHelpLink('UsedBlocks'));
			$return->addElement(new htmlHelpLink('SoftBlockLimit'));
			$return->addElement(new htmlHelpLink('HardBlockLimit'));
			$return->addElement(new htmlHelpLink('GraceBlockPeriod'));
			$return->addElement(new htmlHelpLink('UsedInodes'));
			$return->addElement(new htmlHelpLink('SoftInodeLimit'));
			$return->addElement(new htmlHelpLink('HardInodeLimit'));
			$return->addElement(new htmlHelpLink('GraceInodePeriod'));
			$return->addNewLine();
	
			$i=0;
			// loop for every mointpoint with enabled quotas
			while (isset($this->quota[$server][$i][0])) {
				$return->addElement(new htmlOutputText($this->quota[$server][$i][0]));
				$return->addElement(new htmlOutputText($this->quota[$server][$i][1]));
				$sbLimitInput = new htmlInputField($i . '_2_' . $id, $this->quota[$server][$i][2]);
				$sbLimitInput->setFieldSize(12);
				$sbLimitInput->setFieldMaxLength(20);
				$sbLimitInput->setValidationRule(htmlElement::VALIDATE_NUMERIC);
				$return->addElement($sbLimitInput);
				$hbLimit = new htmlInputField($i . '_3_' . $id, $this->quota[$server][$i][3]);
				$hbLimit->setFieldSize(12);
				$hbLimit->setFieldMaxLength(20);
				$hbLimit->setValidationRule(htmlElement::VALIDATE_NUMERIC);
				$return->addElement($hbLimit);
				$return->addElement(new htmlOutputText($this->quota[$server][$i][4]));
				$return->addElement(new htmlOutputText($this->quota[$server][$i][5]));
				$siLimit = new htmlInputField($i . '_6_' . $id, $this->quota[$server][$i][6]);
				$siLimit->setFieldMaxLength(20);
				$siLimit->setFieldSize(12);
				$siLimit->setValidationRule(htmlElement::VALIDATE_NUMERIC);
				$return->addElement($siLimit);
				$hiLimit = new htmlInputField($i . '_7_' . $id, $this->quota[$server][$i][7]);
				$hiLimit->setFieldMaxLength(20);
				$hiLimit->setFieldSize(12);
				$hiLimit->setValidationRule(htmlElement::VALIDATE_NUMERIC);
				$return->addElement($hiLimit);
				$return->addElement(new htmlOutputText($this->quota[$server][$i][8]));
				$return->addNewLine();
				$i++;
			}
		}
		return $return;
	}
	
	/**
	 * Replaces special characters in HTML name values.
	 *
	 * @param string $input input string
	 * @return string output string
	 */
	function replaceSpecialChars($input) {
		$ret = str_replace(".", "_", $input);
		return $ret;
	}

	/**
	* Returns a list of elements for the account profiles.
	*
	* @return htmlElement profile elements
	*/
	function get_profileOptions() {
		$return = new htmlTable();
		$optionsAvailable = false;
		// get list of lamdaemon servers
		$lamdaemonServers = explode(";", $_SESSION['config']->get_scriptServers());
		for ($s = 0; $s < sizeof($lamdaemonServers); $s++) {
			$temp = explode(":", $lamdaemonServers[$s]);
			$server = $temp[0];
			$id = $this->replaceSpecialChars($server);
			$description = $temp[0];
			if (isset($temp[1])) {
				$description = $temp[1] . ' (' . $temp[0] . ')';
			}
			// Get quotas
			$quotas = lamdaemon(implode(quota::$SPLIT_DELIMITER, array("+", "quota", "get", $this->get_scope())), $server);
			if (sizeof($quotas) == 0) {
				continue;
			}
			$dirs = explode(":", $quotas[0]);
			array_pop($dirs); // remove empty element at the end
			for ($i = 0; $i < sizeof($dirs); $i++) {
				if (strpos($dirs[$i], quota::$QUOTA_PREFIX) !== 0) {
					unset($dirs[$i]);
					$dirs = array_values($dirs);
					$i--;
					continue;
				}
				$dirs[$i] = substr($dirs[$i], strlen(quota::$QUOTA_PREFIX));
				$dirs[$i] = explode(",", $dirs[$i]);
				$dirs[$i] = $dirs[$i][0];
			}
			$dirs = array_values($dirs);
			if (sizeof($dirs) < 1) continue; // stop if no quota directories were found
			$optionsAvailable = true;
			$return->addElement(new htmlSubTitle($description), true);
			$return->addElement(new htmlOutputText('&nbsp;' . _('Mountpoint') . '&nbsp;', false));
			$return->addElement(new htmlOutputText('&nbsp;' . _('Soft block limit') . '&nbsp;', false));
			$return->addElement(new htmlOutputText('&nbsp;' . _('Hard block limit') . '&nbsp;', false));
			$return->addElement(new htmlOutputText('&nbsp;' . _('Soft inode limit') . '&nbsp;', false));
			$return->addElement(new htmlOutputText('&nbsp;' . _('Hard inode limit') . '&nbsp;', false));
			$return->addNewLine();
			$return->addElement(new htmlHelpLink('Mountpoint'));
			$return->addElement(new htmlHelpLink('SoftBlockLimit'));
			$return->addElement(new htmlHelpLink('HardBlockLimit'));
			$return->addElement(new htmlHelpLink('SoftInodeLimit'));
			$return->addElement(new htmlHelpLink('HardInodeLimit'));
			$return->addNewLine();
			
			for ($i = 0; $i < sizeof($dirs); $i++) {
				$return->addElement(new htmlOutputText($dirs[$i]));
				$sbLimit = new htmlInputField("quota_softblock_" . $id . "_" . $dirs[$i]);
				$sbLimit->setFieldSize(12);
				$sbLimit->setFieldMaxLength(20);
				$return->addElement($sbLimit);
				$hbLimit = new htmlInputField("quota_hardblock_" . $id . "_" . $dirs[$i]);
				$hbLimit->setFieldSize(12);
				$hbLimit->setFieldMaxLength(20);
				$return->addElement($hbLimit);
				$siLimit = new htmlInputField("quota_softinode_" . $id . "_" . $dirs[$i]);
				$siLimit->setFieldSize(12);
				$siLimit->setFieldMaxLength(20);
				$return->addElement($siLimit);
				$hiLimit = new htmlInputField("quota_hardinode_" . $id . "_" . $dirs[$i]);
				$hiLimit->setFieldSize(12);
				$hiLimit->setFieldMaxLength(20);
				$return->addElement($hiLimit);
				$return->addNewLine();
			}
		}
		if (!$optionsAvailable) {
			return null;
		}
		return $return;
	}

	/**
	* Checks input values of account profiles.
	*
	* @param array $options a hash array (name => value) containing the options
	* @return array list of error messages (array(type, title, text)) to generate StatusMessages, if any
	*/
	function check_profileOptions($options) {
		$return = array();
		// get list of lamdaemon servers
		$lamdaemonServers = explode(";", $_SESSION['config']->get_scriptServers());
		for ($s = 0; $s < sizeof($lamdaemonServers); $s++) {
			$temp = explode(":", $lamdaemonServers[$s]);
			$server = $temp[0];
			$id = $this->replaceSpecialChars($server);
			// Get quotas
			$quotas = lamdaemon(implode(quota::$SPLIT_DELIMITER, array("+", "quota", "get", $this->get_scope())), $server);
			$dirs = explode(":", $quotas[0]);
			array_pop($dirs); // remove empty element at the end
			for ($i = 0; $i < sizeof($dirs); $i++) {
				if (strpos($dirs[$i], quota::$QUOTA_PREFIX) !== 0) {
					unset($dirs[$i]);
					$i--;
					continue;
				}
				$dirs[$i] = substr($dirs[$i], strlen(quota::$QUOTA_PREFIX));
				$dirs[$i] = explode(",", $dirs[$i]);
				$dirs[$i] = $dirs[$i][0];
			}
			$dirs = array_values($dirs);
			for ($i = 0; $i < sizeof($dirs); $i++) {
				if (!get_preg($options["quota_softblock_" . $id . "_" . $dirs[$i]][0], 'digit')) $return[] = $this->messages['softblock'][0];
				if (!get_preg($options["quota_hardblock_" . $id . "_" . $dirs[$i]][0], 'digit')) $return[] = $this->messages['hardblock'][0];
				if (!get_preg($options["quota_softinode_" . $id . "_" . $dirs[$i]][0], 'digit')) $return[] = $this->messages['softinode'][0];
				if (!get_preg($options["quota_hardinode_" . $id . "_" . $dirs[$i]][0], 'digit')) $return[] = $this->messages['hardinode'][0];
				if (intval($options["quota_softblock_" . $id . "_" . $dirs[$i]][0]) > intval($options["quota_hardblock_" . $id . "_" . $dirs[$i]][0])) $return[] = $this->messages['block_cmp'][0];
				if (intval($options["quota_softinode_" . $id . "_" . $dirs[$i]][0]) > intval($options["quota_hardinode_" . $id . "_" . $dirs[$i]][0])) $return[] = $this->messages['inode_cmp'][0];
				$i++;
			}
		}
		return $return;
	}

	/**
	* Loads the values of an account profile into internal variables.
	*
	* @param array $profile hash array with profile values (identifier => value)
	*/
	function load_profile($profile) {
		$this->initQuotas();
		if (!isset($this->quota) || !is_array($this->quota)) return;
		$servers = array_keys($this->quota);
		for ($s = 0; $s < sizeof($servers); $s++) {
			$server = $servers[$s];
			$id = $this->replaceSpecialChars($server);
			for ($i = 0; $i < sizeof($this->quota[$server]); $i++) {
				$dir = $this->quota[$server][$i][0];
				if (isset($profile["quota_softblock_" . $id . "_" . $dir])) $this->quota[$server][$i][2] = $profile["quota_softblock_" . $id . "_" . $dir][0];
				if (isset($profile["quota_hardblock_" . $id . "_" . $dir])) $this->quota[$server][$i][3] = $profile["quota_hardblock_" . $id . "_" . $dir][0];
				if (isset($profile["quota_softinode_" . $id . "_" . $dir])) $this->quota[$server][$i][6] = $profile["quota_softinode_" . $id . "_" . $dir][0];
				if (isset($profile["quota_hardinode_" . $id . "_" . $dir])) $this->quota[$server][$i][7] = $profile["quota_hardinode_" . $id . "_" . $dir][0];
			}
		}
	}

	/**
	* Returns the PDF entries for this module.
	*
	* @return array list of possible PDF entries
	*/
	function get_pdfEntries() {
		$this->initQuotas();
		if (!isset($this->quota) || !is_array($this->quota)) return array();
		if (sizeof($this->quota) > 0) {
			$quotas = array();
			// get list of lamdaemon servers
			$lamdaemonServers = explode(";", $_SESSION['config']->get_scriptServers());
			for ($s = 0; $s < sizeof($lamdaemonServers); $s++) {
				$temp = explode(":", $lamdaemonServers[$s]);
				$server = $temp[0];
				$description = $server;
				if (isset($temp[1])) $description = $temp[1] . " (" . $server . ")";
				if (!isset($this->quota[$server]) || (sizeof($this->quota[$server]) < 1)) continue;
				
				$quotas[] = '<block><tr>' .
						'<td width="80%"><b>' . $description . '</b></td>' .
						'</tr></block>';
				$quotas[] = '<block><tr>' .
						'<td width="20%"><b>' . _('Mountpoint') . '</b></td>' .
						'<td width="20%"><b>' . _('Soft block') . '</b></td>' .
						'<td width="20%"><b>' . _('Hard block') . '</b></td>' .
						'<td width="20%"><b>' . _('Soft inode') . '</b></td>' .
						'<td width="20%"><b>' . _('Hard inode') . '</b></td></tr></block>';
				for ($i = 0; $i < sizeof($this->quota[$server]); $i++) {
					$quotas[] = '<block><tr>' .
						'<td width="20%" align=\"L\">' . $this->quota[$server][$i][0] . '</td>' .
						'<td width="20%" align=\"L\">' . $this->quota[$server][$i][2] . '</td>' .
						'<td width="20%" align=\"L\">' . $this->quota[$server][$i][3] . '</td>' .
						'<td width="20%" align=\"L\">' . $this->quota[$server][$i][6] . '</td>' .
						'<td width="20%" align=\"L\">' . $this->quota[$server][$i][7] . '</td></tr></block>';
				}
				$quotas[] = '<block><tr><td width="80%"> </td></tr></block>';
			}
			return array('quota_quotas' => $quotas);
		}
		else {
			return array();
		}
	}

	/**
	* Returns an array containing all input columns for the file upload.
	*
	* Syntax:
	* <br> array(
	* <br>  string: name,  // fixed non-translated name which is used as column name (should be of format: <module name>_<column name>)
	* <br>  string: description,  // short descriptive name
	* <br>  string: help,  // help ID
	* <br>  string: example,  // example value
	* <br>  boolean: required  // true, if user must set a value for this column
	* <br> )
	*
	* @param array $selectedModules list of selected account modules
	* @return array column list
	*/
	function get_uploadColumns($selectedModules) {
		$this->initQuotas();
		if (!isset($this->quota) || !is_array($this->quota)) return array();
		$return = array();
		if (sizeof($this->quota) > 0) {
			// get list of lamdaemon servers
			$lamdaemonServers = explode(";", $_SESSION['config']->get_scriptServers());
			for ($s = 0; $s < sizeof($lamdaemonServers); $s++) {
				$temp = explode(":", $lamdaemonServers[$s]);
				$server = $temp[0];
				// Get quotas
				$quotas = lamdaemon(implode(quota::$SPLIT_DELIMITER, array("+", "quota", "get", $this->get_scope())), $server);
				$dirs = explode(":", $quotas[0]);
				array_pop($dirs); // remove empty element at the end
				for ($i = 0; $i < sizeof($dirs); $i++) {
					if (strpos($dirs[$i], quota::$QUOTA_PREFIX) !== 0) {
						unset($dirs[$i]);
						$i--;
						continue;
					}
					$dirs[$i] = substr($dirs[$i], strlen(quota::$QUOTA_PREFIX));
					$dirs[$i] = explode(",", $dirs[$i]);
					$dirs[$i] = $dirs[$i][0];
				}
				$dirs = array_values($dirs);
				for ($i = 0; $i < sizeof($dirs); $i++) {
					$return[] = array(
						'name' => 'quota_' . $server . ':' . $dirs[$i],
						'description' => sprintf(_('Quota for %s on %s'), $dirs[$i], $server),
						'help' => 'upload',
						'example' => '2000,2500,3000,3500');
				}
			}
		}
		return $return;
	}

	/**
	* This function executes one post upload action.
	*
	* @param array $data array containing one account in each element
	* @param array $ids array(<column_name> => <column number>)
	* @param array $failed list of accounts which were not created successfully
	* @param array $temp variable to store temporary data between two post actions
	* @param array $accounts list of LDAP entries
	* @return array current status
	* <br> array (
	* <br>  'status' => 'finished' | 'inProgress'
	* <br>  'progress' => 0..100
	* <br>  'errors' => array (<array of parameters for StatusMessage>)
	* <br> )
	*/
	function doUploadPostActions(&$data, $ids, $failed, &$temp, &$accounts) {
		$errors = array();
		// first call, get list of user names and quota values
		if (!isset($temp['counter'])) {
			$temp['counter'] = 0;
			// create list of quota columns
			$temp['quotas'] = array();
			$columns = array_keys($ids);
			for ($i = 0; $i < sizeof($columns); $i++) {
				if (strpos($columns[$i], 'quota_') === 0) {
					$temp['quotas'][] = substr($columns[$i], 6);
				}
			}
			// select user/group name depending on current scope
			$temp['accounts'] = array();
			$col = 'invalid';
			if ($this->get_scope() == 'user') $col = $ids['posixAccount_userName'];
			elseif ($this->get_scope() == 'group') $col = $ids['posixGroup_cn'];
			// create list of account names and their quota values
			for ($i = 0; $i < sizeof($data); $i++) {
				if (in_array($i, $failed)) continue; // ignore failed accounts
				$name = $data[$i][$col];
				for ($m = 0; $m < sizeof($temp['quotas']); $m++) {
					if ($data[$i][$ids['quota_' . $temp['quotas'][$m]]] != '') {
						$parts = explode(',', $data[$i][$ids['quota_' . $temp['quotas'][$m]]]);
						// check syntax
						if (sizeof($parts) != 4) {
							$errMsg = $this->messages['upload'][0];
							array_push($errMsg, array($i, 'quota_' . $temp['quotas'][$m]));
							$errors[] = $errMsg;
							continue;
						}
						if (!get_preg($parts[0], 'digit')) {
							$errMsg = $this->messages['softblock'][1];
							array_push($errMsg, array($i, 'quota_' . $temp['quotas'][$m]));
							$errors[] = $errMsg;
							continue;
						}
						if (!get_preg($parts[1], 'digit')) {
							$errMsg = $this->messages['hardblock'][1];
							array_push($errMsg, array($i, 'quota_' . $temp['quotas'][$m]));
							$errors[] = $errMsg;
							continue;
						}
						if (!get_preg($parts[2], 'digit')) {
							$errMsg = $this->messages['softinode'][1];
							array_push($errMsg, array($i, 'quota_' . $temp['quotas'][$m]));
							$errors[] = $errMsg;
							continue;
						}
						if (!get_preg($parts[3], 'digit')) {
							$errMsg = $this->messages['hardinode'][1];
							array_push($errMsg, array($i, 'quota_' . $temp['quotas'][$m]));
							$errors[] = $errMsg;
							continue;
						}
						if ($parts[0] > $parts[1]) {
							$errMsg = $this->messages['block_cmp'][1];
							array_push($errMsg, array($i, 'quota_' . $temp['quotas'][$m]));
							$errors[] = $errMsg;
							continue;
						}
						if ($parts[2] > $parts[3]) {
							$errMsg = $this->messages['inode_cmp'][1];
							array_push($errMsg, array($i, 'quota_' . $temp['quotas'][$m]));
							$errors[] = $errMsg;
							continue;
						}
						// save quota settings
						$temp['accounts'][$name][$temp['quotas'][$m]] = $parts;
					}
				}
			}
			return array('status' => 'inProgress', 'progress' => 5, 'errors' => $errors);
		}
		// quotas are ready to set
		elseif ($temp['counter'] < sizeof($temp['accounts'])) {
			$names = array_keys($temp['accounts']);
			$name = $names[$temp['counter']];
			$mountPoints = array_keys($temp['accounts'][$name]);
			// set quota
			for ($m = 0; $m < sizeof($mountPoints); $m++) {
				$mpParts = explode(":", $mountPoints[$m]);
				$server = $mpParts[0];
				$dir = $mpParts[1];
				$quotaString = implode(quota::$SPLIT_DELIMITER, array($name, "quota", "set", $this->get_scope(), $dir . ',' .
					implode(',', $temp['accounts'][$name][$mountPoints[$m]]) . "\n"));
				$result = lamdaemon($quotaString, $server);
				if (is_array($result)) {
					for ($i = 0; $i < sizeof($result); $i++) {
						$parts = explode(",", $result);
						if ($parts[0] == 'ERROR') {
							$errors[] = array('ERROR', $parts[1], $parts[2]);
						}
					}
				}
			}
			// set counters to next account/mount point
			$temp['counter']++;
			return array(
				'status' => 'inProgress',
				'progress' => 5 + (95 * ($temp['counter'] / sizeof($temp['accounts']))),
				'errors' => $errors);
		}
		return array('status' => 'finished');
	}
	
}

?>