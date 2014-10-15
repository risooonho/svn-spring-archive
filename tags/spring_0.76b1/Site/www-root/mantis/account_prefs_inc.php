<?php
	# Mantis - a php based bugtracking system
	# Copyright (C) 2000 - 2002  Kenzaburo Ito - kenito@300baud.org
	# Copyright (C) 2002 - 2004  Mantis Team   - mantisbt-dev@lists.sourceforge.net
	# This program is distributed under the terms and conditions of the GPL
	# See the README and LICENSE files for details

	# --------------------------------------------------------
	# $Id: account_prefs_inc.php,v 1.34 2005/08/10 18:05:16 thraxisp Exp $
	# --------------------------------------------------------

	$t_core_path = config_get( 'core_path' );

	require_once( $t_core_path.'user_pref_api.php' );

	function edit_account_prefs($p_user_id = null, $p_error_if_protected = true, $p_accounts_menu = true, $p_redirect_url = '') {
		if ( null === $p_user_id ) {
			$p_user_id = auth_get_current_user_id();
		}

		$t_redirect_url = $p_redirect_url;
		if ( is_blank( $t_redirect_url ) ) {
			$t_redirect_url = 'account_prefs_page.php';
		}

		# protected account check
		if ( user_is_protected( $p_user_id ) ) {
			if ( $p_error_if_protected ) {
				trigger_error( ERROR_PROTECTED_ACCOUNT, ERROR );
			} else {
				return;
			}
		}

	  if ( ! user_pref_exists( $p_user_id ) ) {
			user_pref_set_default( $p_user_id );
	  }

	    # prefix data with u_
		$t_pref = user_pref_get( $p_user_id );
?>
<?php # Account Preferences Form BEGIN ?>
<br />
<div align="center">
<form method="post" action="account_prefs_update.php">
<input type="hidden" name="user_id" value="<?php echo $p_user_id ?>" />
<input type="hidden" name="redirect_url" value="<?php echo $t_redirect_url ?>" />
<table class="width75" cellspacing="1">
<tr>
	<td class="form-title">
		<?php echo lang_get( 'default_account_preferences_title' ) ?>
	</td>
	<td class="right">
		<?php
			if ( $p_accounts_menu ) {
				print_account_menu( 'account_prefs_page.php' );
			}
		?>
	</td>
</tr>
<tr class="row-1">
	<td class="category" width="50%">
		<?php echo lang_get( 'default_project' ) ?>
	</td>
	<td width="50%">
		<select name="default_project">
			<?php print_project_option_list( $t_pref->default_project ) ?>
		</select>
	</td>
</tr>
<tr class="row-2">
	<td class="category">
		<?php echo lang_get( 'advanced_report' ) ?>
	</td>
	<td>
		<input type="checkbox" name="advanced_report" <?php check_checked( $t_pref->advanced_report, ON ); ?> />
	</td>
</tr>
<tr class="row-1">
	<td class="category">
		<?php echo lang_get( 'advanced_view' ) ?>
	</td>
	<td>
		<input type="checkbox" name="advanced_view" <?php check_checked( $t_pref->advanced_view, ON ); ?> />
	</td>
</tr>
<tr class="row-2">
	<td class="category">
		<?php echo lang_get( 'advanced_update' ) ?>
	</td>
	<td>
		<input type="checkbox" name="advanced_update" <?php check_checked( $t_pref->advanced_update, ON ); ?> />
	</td>
</tr>
<tr class="row-1">
	<td class="category">
		<?php echo lang_get( 'refresh_delay' ) ?>
	</td>
	<td>
		<input type="text" name="refresh_delay" size="4" maxlength="4" value="<?php echo $t_pref->refresh_delay ?>" />
	</td>
</tr>
<tr class="row-2">
	<td class="category">
		<?php echo lang_get( 'redirect_delay' ) ?>
	</td>
	<td>
		<input type="text" name="redirect_delay" size="1" maxlength="1" value="<?php echo $t_pref->redirect_delay ?>" />
	</td>
</tr>
<tr class="row-1">
	<td class="category">
		<?php echo lang_get( 'bugnote_order' ) ?>
	</td>
	<td>
		<input type="radio" name="bugnote_order" value="ASC" <?php check_checked( $t_pref->bugnote_order, 'ASC' ); ?> /><?php echo lang_get( 'bugnote_order_asc' ) ?>
		<input type="radio" name="bugnote_order" value="DESC" <?php check_checked( $t_pref->bugnote_order, 'DESC' ); ?> /><?php echo lang_get( 'bugnote_order_desc' ) ?>
	</td>
</tr>
<?php
	if ( ON == config_get( 'enable_email_notification' ) ) {
?>
<tr class="row-2">
	<td class="category">
		<?php echo lang_get( 'email_on_new' ) ?>
	</td>
	<td>
		<input type="checkbox" name="email_on_new" <?php check_checked( $t_pref->email_on_new, ON ); ?> />
		<?php echo lang_get( 'with_minimum_severity' ) ?>
		<select name="email_on_new_min_severity">
			<option value="<?php echo OFF ?>"><?php echo lang_get( 'any' ) ?></option>
			<option value="<?php echo OFF ?>"></option>
			<?php print_enum_string_option_list( 'severity', $t_pref->email_on_new_min_severity ) ?>
		</select>
	</td>
</tr>
<tr class="row-1">
	<td class="category">
		<?php echo lang_get( 'email_on_assigned' ) ?>
	</td>
	<td>
		<input type="checkbox" name="email_on_assigned" <?php check_checked( $t_pref->email_on_assigned, ON ); ?> />
		<?php echo lang_get( 'with_minimum_severity' ) ?>
		<select name="email_on_assigned_min_severity">
			<option value="<?php echo OFF ?>"><?php echo lang_get( 'any' ) ?></option>
			<option value="<?php echo OFF ?>"></option>
			<?php print_enum_string_option_list( 'severity', $t_pref->email_on_assigned_min_severity ) ?>
		</select>
	</td>
</tr>
<tr class="row-2">
	<td class="category">
		<?php echo lang_get( 'email_on_feedback' ) ?>
	</td>
	<td>
		<input type="checkbox" name="email_on_feedback" <?php check_checked( $t_pref->email_on_feedback, ON ); ?> />
		<?php echo lang_get( 'with_minimum_severity' ) ?>
		<select name="email_on_feedback_min_severity">
			<option value="<?php echo OFF ?>"><?php echo lang_get( 'any' ) ?></option>
			<option value="<?php echo OFF ?>"></option>
			<?php print_enum_string_option_list( 'severity', $t_pref->email_on_feedback_min_severity ) ?>
		</select>
	</td>
</tr>
<tr class="row-1">
	<td class="category">
		<?php echo lang_get( 'email_on_resolved' ) ?>
	</td>
	<td>
		<input type="checkbox" name="email_on_resolved" <?php check_checked( $t_pref->email_on_resolved, ON ); ?> />
		<?php echo lang_get( 'with_minimum_severity' ) ?>
		<select name="email_on_resolved_min_severity">
			<option value="<?php echo OFF ?>"><?php echo lang_get( 'any' ) ?></option>
			<option value="<?php echo OFF ?>"></option>
			<?php print_enum_string_option_list( 'severity', $t_pref->email_on_resolved_min_severity ) ?>
		</select>
	</td>
</tr>
<tr class="row-2">
	<td class="category">
		<?php echo lang_get( 'email_on_closed' ) ?>
	</td>
	<td>
		<input type="checkbox" name="email_on_closed" <?php check_checked( $t_pref->email_on_closed, ON ); ?> />
		<?php echo lang_get( 'with_minimum_severity' ) ?>
		<select name="email_on_closed_min_severity">
			<option value="<?php echo OFF ?>"><?php echo lang_get( 'any' ) ?></option>
			<option value="<?php echo OFF ?>"></option>
			<?php print_enum_string_option_list( 'severity', $t_pref->email_on_closed_min_severity ) ?>
		</select>
	</td>
</tr>
<tr class="row-1">
	<td class="category">
		<?php echo lang_get( 'email_on_reopened' ) ?>
	</td>
	<td>
		<input type="checkbox" name="email_on_reopened" <?php check_checked( $t_pref->email_on_reopened, ON ); ?> />
		<?php echo lang_get( 'with_minimum_severity' ) ?>
		<select name="email_on_reopened_min_severity">
			<option value="<?php echo OFF ?>"><?php echo lang_get( 'any' ) ?></option>
			<option value="<?php echo OFF ?>"></option>
			<?php print_enum_string_option_list( 'severity', $t_pref->email_on_reopened_min_severity ) ?>
		</select>
	</td>
</tr>
<tr class="row-2">
	<td class="category">
		<?php echo lang_get( 'email_on_bugnote_added' ) ?>
	</td>
	<td>
		<input type="checkbox" name="email_on_bugnote" <?php check_checked( $t_pref->email_on_bugnote, ON ); ?> />
		<?php echo lang_get( 'with_minimum_severity' ) ?>
		<select name="email_on_bugnote_min_severity">
			<option value="<?php echo OFF ?>"><?php echo lang_get( 'any' ) ?></option>
			<option value="<?php echo OFF ?>"></option>
			<?php print_enum_string_option_list( 'severity', $t_pref->email_on_bugnote_min_severity ) ?>
		</select>
	</td>
</tr>
<tr class="row-1">
	<td class="category">
		<?php echo lang_get( 'email_on_status_change' ) ?>
	</td>
	<td>
		<input type="checkbox" name="email_on_status" <?php check_checked( $t_pref->email_on_status, ON ); ?> />
		<?php echo lang_get( 'with_minimum_severity' ) ?>
		<select name="email_on_status_min_severity">
			<option value="<?php echo OFF ?>"><?php echo lang_get( 'any' ) ?></option>
			<option value="<?php echo OFF ?>"></option>
			<?php print_enum_string_option_list( 'severity', $t_pref->email_on_status_min_severity ) ?>
		</select>
	</td>
</tr>
<tr class="row-2">
	<td class="category">
		<?php echo lang_get( 'email_on_priority_change' ) ?>
	</td>
	<td>
		<input type="checkbox" name="email_on_priority" <?php check_checked( $t_pref->email_on_priority , ON); ?> />
		<?php echo lang_get( 'with_minimum_severity' ) ?>
		<select name="email_on_priority_min_severity">
			<option value="<?php echo OFF ?>"><?php echo lang_get( 'any' ) ?></option>
			<option value="<?php echo OFF ?>"></option>
			<?php print_enum_string_option_list( 'severity', $t_pref->email_on_priority_min_severity ) ?>
		</select>
	</td>
</tr>
<tr class="row-1">
	<td class="category">
		<?php echo lang_get( 'email_bugnote_limit' ) ?>
	</td>
	<td>
		<input type="text" name="email_bugnote_limit" maxlength="2" size="2" value="<?php echo $t_pref->email_bugnote_limit ?>">
	</td>
</tr>
<?php } else { ?>
		<input type="hidden" name="email_on_new"      value="<?php echo $t_pref->email_on_new ?>" />
		<input type="hidden" name="email_on_assigned" value="<?php echo $t_pref->email_on_assigned ?>" />
		<input type="hidden" name="email_on_feedback" value="<?php echo $t_pref->email_on_feedback ?>" />
		<input type="hidden" name="email_on_resolved" value="<?php echo $t_pref->email_on_resolved ?>" />
		<input type="hidden" name="email_on_closed"   value="<?php echo $t_pref->email_on_closed ?>" />
		<input type="hidden" name="email_on_reopened" value="<?php echo $t_pref->email_on_reopened ?>" />
		<input type="hidden" name="email_on_bugnote"  value="<?php echo $t_pref->email_on_bugnote ?>" />
		<input type="hidden" name="email_on_status"   value="<?php echo $t_pref->email_on_status ?>" />
		<input type="hidden" name="email_on_priority" value="<?php echo $t_pref->email_on_priority ?>" />
		<input type="hidden" name="email_on_new_min_severity"      value="<?php echo $t_pref->email_on_new_min_severity ?>" />
		<input type="hidden" name="email_on_assigned_min_severity" value="<?php echo $t_pref->email_on_assigned_min_severity ?>" />
		<input type="hidden" name="email_on_feedback_min_severity" value="<?php echo $t_pref->email_on_feedback_min_severity ?>" />
		<input type="hidden" name="email_on_resolved_min_severity" value="<?php echo $t_pref->email_on_resolved_min_severity ?>" />
		<input type="hidden" name="email_on_closed_min_severity"   value="<?php echo $t_pref->email_on_closed_min_severity ?>" />
		<input type="hidden" name="email_on_reopened_min_severity" value="<?php echo $t_pref->email_on_reopened_min_severity ?>" />
		<input type="hidden" name="email_on_bugnote_min_severity"  value="<?php echo $t_pref->email_on_bugnote_min_severity ?>" />
		<input type="hidden" name="email_on_status_min_severity"   value="<?php echo $t_pref->email_on_status_min_severity ?>" />
		<input type="hidden" name="email_on_priority_min_severity" value="<?php echo $t_pref->email_on_priority_min_severity ?>" />
		<input type="hidden" name="email_bugnote_limit" value="<?php echo $t_pref->email_bugnote_limit ?>" />
<?php } ?>
<tr class="row-2">
	<td class="category">
		<?php echo lang_get( 'language' ) ?>
	</td>
	<td>
		<select name="language">
			<?php print_language_option_list( $t_pref->language ) ?>
		</select>
	</td>
</tr>
<tr>
	<td colspan="2" class="center">
		<input type="submit" class="button" value="<?php echo lang_get( 'update_prefs_button' ) ?>" />
	</td>
</tr>
</table>
</form>
</div>

<br />

<div class="border-center">
	<form method="post" action="account_prefs_reset.php">
	<input type="hidden" name="user_id" value="<?php echo $p_user_id ?>" />
	<input type="hidden" name="redirect_url" value="<?php echo $t_redirect_url ?>" />
	<input type="submit" class="button" value="<?php echo lang_get( 'reset_prefs_button' ) ?>" />
	</form>
</div>

<?php
	} # end of edit_account_prefs()
?>