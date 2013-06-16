<?php
/**
 * @file
 * widget.tpl.php
 *
 * Plain widget theme for Vote Up/Down
 */
?>
<?php if ($show_links): ?>
  <div class="vud-widget vud-widget-stalkerspace" id="<?php print $id; ?>">
    <?php if ($show_up_as_link): ?>
      <a href="<?php print $link_up; ?>" rel="nofollow" class="<?php print $link_class_up; ?>">
      <?php $text = t('Vote up!'); ?>
    <?php elseif ($show_down_as_link && $show_reset): ?>
      <a href="<?php print $link_reset; ?>" rel="nofollow" class="<?php print $link_class_reset; ?>">
      <?php $text = t('Reset'); ?>
    <?php endif; ?>
    <span class="<?php print $class_up; ?>" title="<?php print $text; ?>"></span>
    <div class="element-invisible"><?php print $text; ?></div>
    <?php if ($show_up_as_link|| $show_reset) : ?>
      </a>
    <?php endif; ?>
    <?php if ($show_down_as_link): ?>
      <a href="<?php print $link_down; ?>" rel="nofollow" class="<?php print $link_class_down; ?>">
        <?php $text = t('Vote down!'); ?>
        <?php elseif ($show_up_as_link && $show_reset): ?>
      <a href="<?php print $link_reset; ?>" rel="nofollow" class="<?php print $link_class_reset; ?>">
      <?php $text = t('Reset'); ?>
      <?php endif; ?>
      <span class="<?php print $class_down; ?>" title="<?php print $text; ?>"></span>
      <div class="element-invisible"><?php print $text; ?></div>
    <?php if ($show_down_as_link || $show_reset): ?>
      </a>
    <?php endif; ?>
  </div>
<?php endif; ?>
