(function ($) {
  Drupal.behaviors.subdomain = {
    attach: function(context, settings) {
      $('#edit-subdomain-mode').once('subdomain-mode', function() {
        $(this).change(function() {
          $('#edit-subdomain-source option[value="default"]').text(settings.subdomain.sourceOptionText[$(this).val()]);
        }).change();
      });
      
      // Add JS validation for subdomain selection
      var timer;
      var selector = Drupal.settings.subdomain ? Drupal.settings.subdomain.selector : undefined;
      
      if (selector != undefined) {
        $('#'+selector+':not(.processed)')
        .addClass('processed')
        .after('<span id="subdomain-check" style="display:none;"></span>')
        .keyup(function(e) {
          $('#subdomain-check').hide();
          var v = $(this).val();
          clearTimeout(timer);
          timer = setTimeout(function() {
            Drupal.subdomainValidate(v);
          }, 500);
        });
      }
    }
  }
  
  Drupal.subdomainValidate = function(check) {
    if (check) {
      $('#subdomain-check')
        .text('Checking...')
        .removeClass('malformed')
        .removeClass('duplicate')
        .addClass('checking')
        .show();
      
      $.get('/subdomain/validate', {subdomain:check,sid:Drupal.settings.subdomain.sid},function(data) {
        if (data.malformed) {
          $('#subdomain-check')
            .text('Not a valid subdomain! A subdomain can only contain A through Z, 0 through 9, and dashes.')
            .removeClass('checking')
            .addClass('malformed')
            .show();
        }
        else {
          if (data.available) {
            $('#subdomain-check')
              .text('Available!')
              .removeClass('checking')
              .removeClass('duplicate')
              .show();
          }
          else {
            $('#subdomain-check')
              .text('Not available!')
              .removeClass('checking')
              .addClass('duplicate')
              .show();
          }
        }
      }, 'json');
    }
    else {
      $('#subdomain-check').hide();
    }
  }
})(jQuery);