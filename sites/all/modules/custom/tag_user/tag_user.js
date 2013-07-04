(function ($) {
    /**
     * Attaches the autocomplete behavior to all required fields.
     */
    Drupal.behaviors.tag_user = {
        attach: function (context, settings) {
            var acdb = [];
            $('input.tag-user', context).once('tag-user', function () {
                var tag_user = new Drupal.tag_user();
                var uri = this.value;
                if (!acdb[uri]) {
                    acdb[uri] = new Drupal.tag_userACDB(uri);
                }
                var $input = $('#' + this.id.substr(0, this.id.length - 13))
                    .attr('autocomplete', 'OFF')
                    .attr('aria-autocomplete', 'list');

                $($input.form).submit(Drupal.tag_userSubmit);
                $input.parent()
                    .attr('role', 'application')
                    .append($('<span class="element-invisible" aria-live="assertive"></span>')
                        .attr('id', $input.attr('id') + '-autocomplete-aria-live')
                    );
                new Drupal.tag_userAC($input, acdb[uri], tag_user);
            });
        }
    };

    /**
     * Prevents the form from submitting if the suggestions popup is open
     * and closes the suggestions popup when doing so.
     */
    Drupal.tag_userSubmit = function () {
        return $('#autocomplete').each(function () {
            this.owner.hidePopup();
        }).length == 0;
    };

    /**
     * An AutoComplete object.
     */
    Drupal.tag_userAC = function ($input, db, tag_user) {
        var ac = this;
        this.input = $input;
        this.ariaLive = $('#' + this.input.id + '-autocomplete-aria-live');
        this.db = db;
        this.tag_user = tag_user;

        $input
            .keydown(function (event) { return ac.onkeydown(this, event); })
            .keyup(function (event) { ac.onkeyup(this, event); })
            .blur(function () { ac.hidePopup(); ac.db.cancel(); });

    };

    /**
     * Handler for the "keydown" event.
     */
    Drupal.tag_userAC.prototype.onkeydown = function (input, e) {
        if (!e) {
            e = window.event;
        }
        switch (e.keyCode) {
            case 40: // down arrow.
                this.selectDown();
                return false;
            case 38: // up arrow.
                this.selectUp();
                return false;
            case 13: // enter
                if (input.value.length > 0 && !input.readOnly) {
                    if (this.tag_user.tagging) {
                        this.hidePopup(e.keyCode);
                        return false;
                    }
                }

            default: // All other keys.
                return true;
        }
    };

    /**
     * Handler for the "keyup" event.
     */
    Drupal.tag_userAC.prototype.onkeyup = function (input, e) {
        if (!e) {
            e = window.event;
        }
        switch (e.keyCode) {
            case 16: // Shift.
            case 17: // Ctrl.
            case 18: // Alt.
            case 20: // Caps lock.
            case 33: // Page up.
            case 34: // Page down.
            case 35: // End.
            case 36: // Home.
            case 37: // Left arrow.
            case 38: // Up arrow.
            case 39: // Right arrow.
            case 40: // Down arrow.
                return true;

            case 27: // Esc.
                this.hidePopup(e.keyCode);
            case 9:  // Tab.
            case 13: // Enter.
                return true;

            default: // All other keys.
                // Get the cursor position and check if 30 chars before contains
                // a @ or + (user defined eventually)
                var cursor = input.selectionStart;
                var last30 = input.value.substr((Math.max(cursor - 30, 0)), cursor);
                var tags = Drupal.settings.tag_user_tag_symbols; // TODO allow the user to change this
                tags = tags.split(",");
                for (var key in tags) {
                    if (last30.indexOf(" " + tags[key]) > 0 || last30.indexOf(tags[key]) == 0) {
                        // Find the matched tag, ensure it has a space beforehand
                        // Then the name is what's after that.
                        this.tag_user.tagged_user = last30.substr(last30.indexOf(tags[key]));
                        this.tag_user.tagging = true;
                    }
                }
                this.populatePopup();

                return true;
        }
    };

    /**
     * Puts the currently highlighted suggestion into the autocomplete field.
     */
    Drupal.tag_userAC.prototype.select = function (node) {
        this.input.value = $(node).data('autocompleteValue');
    };

    /**
     * Highlights the next suggestion.
     */
    Drupal.tag_userAC.prototype.selectDown = function () {
        if (this.selected && this.selected.nextSibling) {
            this.highlight(this.selected.nextSibling);
        }
        else if (this.popup) {
            var lis = $('li', this.popup);
            if (lis.length > 0) {
                this.highlight(lis.get(0));
            }
        }
    };

    /**
     * Highlights the previous suggestion.
     */
    Drupal.tag_userAC.prototype.selectUp = function () {
        if (this.selected && this.selected.previousSibling) {
            this.highlight(this.selected.previousSibling);
        }
    };

    /**
     * Highlights a suggestion.
     */
    Drupal.tag_userAC.prototype.highlight = function (node) {
        if (this.selected) {
            $(this.selected).removeClass('selected');
        }
        $(node).addClass('selected');
        this.selected = node;
        $(this.ariaLive).html($(this.selected).html());
    };

    /**
     * Unhighlights a suggestion.
     */
    Drupal.tag_userAC.prototype.unhighlight = function (node) {
        $(node).removeClass('selected');
        this.selected = false;
        $(this.ariaLive).empty();
    };

    /**
     * Hides the autocomplete suggestions.
     */
    Drupal.tag_userAC.prototype.hidePopup = function (keycode) {
        if (this.selected && ((keycode && keycode != 46 && keycode != 8 && keycode != 27) || !keycode)) {
            var text = this.input[0].value;
            text = text.replace(this.tag_user.tagged_user, $(this.selected).data('autocompleteValue'));
            this.input[0].value = text;
            this.tag_user.tagged_user = "";
            this.tag_user.tagging = false;
        }
        // Hide popup.
        var popup = this.popup;
        if (popup) {
            this.popup = null;
            $(popup).fadeOut('fast', function () { $(popup).remove(); });
        }
        this.selected = false;
        $(this.ariaLive).empty();
    };

    /**
     * Positions the suggestions popup and starts a search.
     */
    Drupal.tag_userAC.prototype.populatePopup = function () {
        //if (this.tag_user.tagging == true) {
            var $input = $(this.input);
            var position = $input.position();
            // Show popup.
            if (this.popup) {
                $(this.popup).remove();
            }
            this.selected = false;
            this.popup = $('<div id="autocomplete"></div>')[0];
            this.popup.owner = this;
            $(this.popup).css({
                top: parseInt(position.top + this.input.offsetHeight, 10) + 'px',
                left: parseInt(position.left, 10) + 'px',
                width: $input.innerWidth() + 'px',
                display: 'none'
            });
            $input.before(this.popup);

            // Do search.
            this.db.owner = this;
            this.db.search(this.tag_user.tagged_user);
        //}
    };

    /**
     * Fills the suggestion popup with any matches received.
     */
    Drupal.tag_userAC.prototype.found = function (matches) {

        // Prepare matches.
        var ul = $('<ul></ul>');
        var ac = this;
        for (key in matches) {
            $('<li></li>')
                .html($('<div></div>').html(matches[key]))
                .mousedown(function () { ac.select(this); })
                .mouseover(function () { ac.highlight(this); })
                .mouseout(function () { ac.unhighlight(this); })
                .data('autocompleteValue', key)
                .appendTo(ul);
        }

        // Show popup with matches, if any.
        if (this.popup) {
            if (ul.children().length) {
                $(this.popup).empty().append(ul).show();
                $(this.ariaLive).html(Drupal.t('Autocomplete popup'));
            }
            else {
                $(this.popup).css({ visibility: 'hidden' });
                this.hidePopup();
            }
        }
    };

    Drupal.tag_userAC.prototype.setStatus = function (status) {
        switch (status) {
            case 'begin':
                $(this.input).addClass('throbbing');
                $(this.ariaLive).html(Drupal.t('Searching for matches...'));
                break;
            case 'cancel':
            case 'error':
            case 'found':
                $(this.input).removeClass('throbbing');
                break;
        }
    };

    /**
     * A Tag User object.
     */
    Drupal.tag_user = function () {
        this.tagging = false;
        this.tagged_user = "";
    };

    /**
     * An AutoComplete DataBase object.
     */
    Drupal.tag_userACDB = function (uri) {
        this.uri = uri;
        this.delay = 300;
        this.cache = {};
    };

    /**
     * Performs a cached and delayed search.
     */
    Drupal.tag_userACDB.prototype.search = function (searchString) {
        var db = this;

        // Slice off the @ / + from the front
        this.searchString = searchString.slice(1);

        // See if this string needs to be searched for anyway.
        searchString = this.searchString.replace(/^\s+|\s+$/, '');

        if (searchString.length <= 0 ||
            searchString.charAt(searchString.length - 1) == ',') {
            return;
        }

        // See if this key has been searched for before.
        if (this.cache[searchString]) {
            return this.owner.found(this.cache[searchString]);
        }

        // Initiate delayed search.
        if (this.timer) {
            clearTimeout(this.timer);
        }
        this.timer = setTimeout(function () {
            db.owner.setStatus('begin');

            // Ajax GET request for autocompletion. We use Drupal.encodePath instead of
            // encodeURIComponent to allow autocomplete search terms to contain slashes.
            $.ajax({
                type: 'GET',
                url: db.uri + '/' + Drupal.encodePath(searchString),
                dataType: 'json',
                success: function (matches) {
                    if (typeof matches.status == 'undefined' || matches.status != 0) {
                        db.cache[searchString] = matches;
                        // Verify if these are still the matches the user wants to see.
                        if (db.searchString == searchString) {
                            db.owner.found(matches);
                        }
                        db.owner.setStatus('found');
                    }
                },
                error: function (xmlhttp) {
                    alert(Drupal.ajaxError(xmlhttp, db.uri));
                }
            });
        }, this.delay);
    };

    /**
     * Cancels the current autocomplete request.
     */
    Drupal.tag_userACDB.prototype.cancel = function () {
        if (this.owner) this.owner.setStatus('cancel');
        if (this.timer) clearTimeout(this.timer);
        this.searchString = '';
    };

})(jQuery);
