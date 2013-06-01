<?php

/**
 * @file
 */

function stalkerspace_admin_page() {
  $form['stalkerspace'] = array(
    '#title' => 'Placeholder',
    '#type' => 'checkbox',
    '#description' => 'this currently does nothing',
  );

  return $form;
}
