<?php

/**
 * @file
 */

function stalkerspace_admin_page() {
  $form['stalkerspace_confession_title'] = array(
    '#title' => t('Confession Title'),
    '#type' => 'checkbox',
    '#description' => t('Alter the title of the block on all group pages prompting users to post'),
    '#default_value' => variable_get('stalkerspace_confession_title', t('Confess'))
  );

  return system_settings_form($form);
}
