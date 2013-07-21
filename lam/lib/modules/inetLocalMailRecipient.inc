<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2004 - 2013  Roland Gruber

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
* Provides mail routing for user accounts.
*
* @package modules
* @author Roland Gruber
*/

/**
* Provides mail routing for users.
*
* @package modules
*/
class inetLocalMailRecipient extends baseModule {

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
		$return['icon'] = 'mailBig.png';
		// manages host accounts
		$return["account_types"] = array("user");
		// alias name
		$return["alias"] = _("Mail routing");
		// module dependencies
		$return['dependencies'] = array('depends' => array(), 'conflicts' => array());
		// managed object classes
		$return['objectClasses'] = array('inetLocalMailRecipient');
		// managed attributes
		$return['attributes'] = array('mailLocalAddress', 'mailHost', 'mailRoutingAddress');
		// help Entries
		$return['help'] = array(
			'routingAdr' => array(
				"Headline" => _("Routing address"), 'attr' => 'mailRoutingAddress',
				"Text" => _("This is the target email address for the user's mails.")
			),
			'localAdr' => array(
				"Headline" => _("Local address"), 'attr' => 'mailLocalAddress',
				"Text" => _("This is one of the users public email addresses.")
			),
			'localAdrList' => array(
				"Headline" => _("Local address list"), 'attr' => 'mailLocalAddress',
				"Text" => _("This is a comma separated list of the users public email addresses.")
			),
			'host' => array(
				"Headline" => _("Mail server"), 'attr' => 'mailHost',
				"Text" => _("This is the mail server for the user.")
			));
		// profile options
		$profileContainer = new htmlTable();
		$profileContainer->addElement(new htmlTableExtendedInputField(_('Mail server'), 'inetLocalMailRecipient_host', null, 'host'));
		$return['profile_options'] = $profileContainer;
		// profile checks
		$return['profile_checks']['inetLocalMailRecipient_host'] = array(
			'type' => 'ext_preg',
			'regex' => 'DNSname',
			'error_message' => $this->messages['host'][0]);
		// profile mappings
		$return['profile_mappings'] = array(
			'inetLocalMailRecipient_host' => 'mailHost'
		);
		// upload fields
		$return['upload_columns'] = array(
			array(
				'name' => 'inetLocalMailRecipient_routingAdr',
				'description' => _('Routing address'),
				'help' => 'routingAdr',
				'example' => _('smiller@otherdomain.org')
			),
			array(
				'name' => 'inetLocalMailRecipient_localAdr',
				'description' => _('Local address list'),
				'help' => 'localAdrList',
				'example' => _('smiller@yourdomain.org')
			),
			array(
				'name' => 'inetLocalMailRecipient_server',
				'description' => _('Mail server'),
				'help' => 'host',
				'example' => _('mail.yourdomain.org')
			)
		);
		// available PDF fields
		$return['PDF_fields'] = array(
			'routingAdr' => _('Routing address'),
			'localAdr' => _('Local address list'),
			'host' => _('Mail server')
		);
		return $return;
	}

	/**
	* This function fills the error message array with messages
	*/
	function load_Messages() {
		$this->messages['routingAdr'][0] = array('ERROR', 'Routing address is invalid!');  // third array value is set dynamically
		$this->messages['routingAdr'][1] = array('ERROR', _('Account %s:') . ' inetLocalMailRecipient_routingAdr', 'Routing address is invalid!');
		$this->messages['localAdr'][0] = array('ERROR', 'Local address is invalid!');  // third array value is set dynamically
		$this->messages['localAdr'][1] = array('ERROR', _('Account %s:') . ' inetLocalMailRecipient_localAdr', 'Local address is invalid!');
		$this->messages['host'][0] = array('ERROR', 'Mail server is invalid!');  // third array value is set dynamically
		$this->messages['host'][1] = array('ERROR', _('Account %s:') . ' inetLocalMailRecipient_server', 'Mail server is invalid!');
	}

	/**
	 * Returns the HTML meta data for the main account page.
	 * 
	 * @return htmlElement HTML meta data
	 */
	function display_html_attributes() {
		$return = new htmlTable();
		// mail routing address
		$routingAddress = '';
		if (isset($this->attributes['mailRoutingAddress'][0])) $routingAddress = $this->attributes['mailRoutingAddress'][0];
		$return->addElement(new htmlTableExtendedInputField(_('Routing address'), 'routingAdr', $routingAddress, 'routingAdr'), true);
		// mail server
		$mailServer = '';
		if (isset($this->attributes['mailHost'][0])) $mailServer = $this->attributes['mailHost'][0];
		$return->addElement(new htmlTableExtendedInputField(_('Mail server'), 'host', $mailServer, 'host'), true);
		// list current local addresses
		$localAdresses = array();
		if (isset($this->attributes['mailLocalAddress'])) $localAdresses = $this->attributes['mailLocalAddress'];
		for ($i = 0; $i < sizeof($localAdresses); $i++) {
			$return->addElement(new htmlOutputText(_('Local address')));
			$return->addElement(new htmlInputField('localAdr' . $i, $localAdresses[$i]));
			$return->addElement(new htmlButton('delAdr' . $i, 'del.png', true));
			$return->addElement(new htmlHelpLink('localAdr'), true);
		}
		// input box for new local addresses
		$return->addElement(new htmlOutputText(_('New local address')));
		$return->addElement(new htmlInputField('localAdr', ''));
		$return->addElement(new htmlButton('addAdr', 'add.png', true));
		$return->addElement(new htmlHelpLink('localAdr'));
		$return->addElement(new htmlHiddenInput('adr_number', sizeof($localAdresses)));
		return $return;
	}

	/**
	* Processes user input of the primary module page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @return array list of info/error messages
	*/
	function process_attributes() {
		$errors = array();
		$this->attributes['mailRoutingAddress'] = array();
		$this->attributes['mailLocalAddress'] = array();
		$this->attributes['mailHost'] = array();
		// check routing address
		if (isset($_POST['routingAdr']) && ($_POST['routingAdr'] != "")) {
			// check if address has correct format
			if (get_preg($_POST['routingAdr'], 'email')) {
				$this->attributes['mailRoutingAddress'][0] = $_POST['routingAdr'];
			}
			else {
					$message = $this->messages['routingAdr'][0];
					$message[] = $_POST['routingAdr'];
					$errors[] = $message;
			}
		}
		// check mail server
		if (isset($_POST['host']) && ($_POST['host'] != "")) {
			// check if address has correct format
			if (get_preg($_POST['host'], 'DNSname')) {
				$this->attributes['mailHost'][0] = $_POST['host'];
			}
			else {
					$message = $this->messages['host'][0];
					$message[] = $_POST['host'];
					$errors[] = $message;
			}
		}
		// check old local addresses
		if (isset($_POST['adr_number'])) {
			for ($i = 0; $i < $_POST['adr_number']; $i++) {
				if (isset($_POST['delAdr' . $i])) continue;
				if (isset($_POST['localAdr' . $i]) && ($_POST['localAdr' . $i] != "")) {
					// check if address has correct format
					if (!get_preg($_POST['localAdr' . $i], 'mailLocalAddress')) {
						$message = $this->messages['localAdr'][0];
						$message[] = $_POST['localAdr' . $i];
						$errors[] = $message;
					}
					$this->attributes['mailLocalAddress'][] = $_POST['localAdr' . $i];
				}
			}
		}
		// check new local address
		if (isset($_POST['localAdr']) && ($_POST['localAdr'] != "")) {
			// check if address has correct format
			if (get_preg($_POST['localAdr'], 'mailLocalAddress')) {
				// check if new address is not already in database
				$data = searchLDAPByAttribute('mailLocalAddress', $_POST['localAdr'], 'inetLocalMailRecipient', array('dn'), array('user'));
				if (sizeof($data) > 0) {
					$errors[] = array('WARN', _('This mail address is already in use:') . " " . $_POST['localAdr'], $data[0]['dn']);
				}
				$this->attributes['mailLocalAddress'][] = $_POST['localAdr'];
			}
			else {
					$message = $this->messages['localAdr'][0];
					$message[] = $_POST['localAdr'];
					$errors[] = $message;
			}
		}
		$this->attributes['mailLocalAddress'] = array_unique($this->attributes['mailLocalAddress']);
		return $errors;
	}

	/**
	* In this function the LDAP account is built up.
	*
	* @param array $rawAccounts list of hash arrays (name => value) from user input
	* @param array $ids list of IDs for column position (e.g. "posixAccount_uid" => 5)
	* @param array $partialAccounts list of hash arrays (name => value) which are later added to LDAP
	* @param array $selectedModules list of selected account modules
	* @return array list of error messages if any
	*/
	function build_uploadAccounts($rawAccounts, $ids, &$partialAccounts, $selectedModules) {
		$messages = array();
		for ($i = 0; $i < sizeof($rawAccounts); $i++) {
			// add object class
			if (!in_array("inetLocalMailRecipient", $partialAccounts[$i]['objectClass'])) $partialAccounts[$i]['objectClass'][] = "inetLocalMailRecipient";
			// add local addresses
			if ($rawAccounts[$i][$ids['inetLocalMailRecipient_localAdr']] != "") {
				$adrs = explode(',', $rawAccounts[$i][$ids['inetLocalMailRecipient_localAdr']]);
				// check format
				for ($a = 0; $a < sizeof($adrs); $a++) {
					if (get_preg($adrs[$a], 'mailLocalAddress')) {
						$partialAccounts[$i]['mailLocalAddress'][] = $adrs[$a];
					}
					else {
						$errMsg = $this->messages['localAdr'][1];
						array_push($errMsg, array($i));
						$messages[] = $errMsg;
					}
				}
			}
			// add routing address
			if ($rawAccounts[$i][$ids['inetLocalMailRecipient_routingAdr']] != "") {
				// check format
				if (get_preg($rawAccounts[$i][$ids['inetLocalMailRecipient_routingAdr']], 'email')) {
					$partialAccounts[$i]['mailRoutingAddress'][] = $rawAccounts[$i][$ids['inetLocalMailRecipient_routingAdr']];
				}
				else {
					$errMsg = $this->messages['routingAdr'][1];
					array_push($errMsg, array($i));
					$messages[] = $errMsg;
				}
			}
			// add mail server
			if ($rawAccounts[$i][$ids['inetLocalMailRecipient_server']] != "") {
				// check format
				if (get_preg($rawAccounts[$i][$ids['inetLocalMailRecipient_server']], 'DNSname')) {
					$partialAccounts[$i]['mailHost'][] = $rawAccounts[$i][$ids['inetLocalMailRecipient_server']];
				}
				else {
					$errMsg = $this->messages['host'][1];
					array_push($errMsg, array($i));
					$messages[] = $errMsg;
				}
			}
		}
		return $messages;
	}

	/**
	* Returns the PDF entries for this module.
	*
	* @return array list of possible PDF entries
	*/
	function get_pdfEntries() {
		$return = array();
		$this->addSimplePDFField($return, 'routingAdr', _('Routing address'), 'mailRoutingAddress');
		$this->addSimplePDFField($return, 'localAdr', _('Local address list'), 'mailLocalAddress');
		$this->addSimplePDFField($return, 'host', _('Mail server'), 'mailHost');
		return $return;
	}

}


?>