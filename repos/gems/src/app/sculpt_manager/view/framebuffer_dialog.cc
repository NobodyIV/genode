/*
 * \brief  Framebuffer management dialog
 * \author Roman Iten
 * \date   2018-08-18
 */

/* Genode includes */
#include <base/log.h>

/* local includes */
#include "framebuffer_dialog.h"


void Sculpt::Framebuffer_dialog::_gen_mode(Xml_generator           &xml,
                                           Fb_connector const &connector,
                                           Fb_connector_mode const &mode,
                                           bool                     hz) const
{
	gen_named_node(xml, "hbox", mode.id(), [&] () {

		gen_named_node(xml, "float", "left", [&] () {
			xml.attribute("west", "yes");

			xml.node("hbox", [&] () {
				gen_named_node(xml, "button", "button", [&] () {

					if (_mode_item.hovered(mode.id()))
						xml.attribute("hovered", "yes");

					// FIXME: that if condition is not what we want
					//if (_mode_item.selected(mode.id()))
					// Let's try it with this one:
					//if (connector.enabled && _mode_item.Id == connector.selected_mode().Id)
					if (connector.enabled && mode.id() == connector.selected_mode().id())
						xml.attribute("selected", "yes");

					xml.node("label", [&] () {
						xml.attribute("text", " "); });
				});

				gen_named_node(xml, "label", "label", [&] () {
					xml.attribute("text", String<16>(" ", mode.width, "x", mode.height)); });
			});
		});

		if (hz) {
			gen_named_node(xml, "float", "right", [&] () {
				xml.attribute("east", "yes");
				xml.node("label", [&] () {
					xml.attribute("text", String<64>(mode.hz, " hz"));
				});
			});
    	}
	});
}

/*
void Sculpt::Framebuffer_dialog::_gen_common_resolutions(Xml_generator &xml) const
{
	Fb_connector::Name connector_name { "Common Resolutions" };
	bool const selected = _connector_item.selected(connector_name);

	// We can't have the same "XML path" for the common resolution and
	// for the individual connectors, otherwise our .match() function
	// will not work as intended.
	// Leave the Common Resolution feature for later.
	gen_named_node(xml, "button", connector_name, [&] () {

		if (_connector_item.hovered(connector_name))
			xml.attribute("selected", "yes");

		if (selected)
			xml.attribute("selected", "yes");

		xml.node("hbox", [&] () {
			gen_named_node(xml, "float", "info", [&] () {
				xml.attribute("west", "yes");

				xml.node("hbox", [&] () {

					gen_named_node(xml, "label", "device", [&] () {
						xml.attribute("text", connector_name); });
				});
			});
		});
	});

	if (selected) {
		gen_named_node(xml, "frame", connector_name, [&] () {
			xml.node("vbox", [&] () {
				_fb_connectors.for_each([&] (Fb_connector const& connector) {
					connector.modes.for_each([&] (Fb_connector_mode const &mode) {
						if (mode.common(_fb_connectors)) {
							_gen_mode(xml, mode, true);
						}
					});
				});
			});
		});
	}
}*/


void Sculpt::Framebuffer_dialog::_gen_connector(Xml_generator      &xml,
                                                Fb_connector const &connector) const
{
	bool const selected = _connector_item.selected(connector.name);

	gen_named_node(xml, "button", connector.name, [&] () {

		if (_connector_item.hovered(connector.name) && connector.connected)
			xml.attribute("hovered", "yes");

		if (selected && connector.connected)
			xml.attribute("selected", "yes");

		xml.node("hbox", [&] () {
			gen_named_node(xml, "float", "info", [&] () {
				xml.attribute("west", "yes");

				xml.node("hbox", [&] () {

					gen_named_node(xml, "label", "device", [&] () {
						xml.attribute("text", connector.name);
					});

				});
			});

			gen_named_node(xml, "float", "connected", [&] () {
				xml.attribute("east", "yes");
				xml.node("label", [&] () {
  					xml.attribute("text", connector.connected ? "connected" : "");
				});
			});
		});
	});

	// If the connector isn't connected, we don't want to spawn an empty hbox.
	// connector.modes is a List_model, which only exposes an iterator and
	// no is_empty() function. We want something similar to:
	//if (selected && connector.modes != {}) {
	// Instead of modifying the underlying data structure, we'll hack around it:

	if (selected) {
		if (connector.connected) {
		xml.node("frame", [&] () {
			xml.attribute("name", connector.name);
			
				xml.node("vbox", [&] () {
					connector.modes.for_each([&] (Fb_connector_mode const &mode) {
						_gen_mode(xml, connector, mode, true);
					});
				});
			
		});
		}
	}
}


void Sculpt::Framebuffer_dialog::generate(Xml_generator &xml) const
{
	gen_named_node(xml, "frame", "framebuffer", [&] () {
		xml.node("vbox", [&] () {
			gen_named_node(xml, "label", "title", [&] () {
				xml.attribute("text", "Display");
				xml.attribute("font", "title/regular");
			});

			// The common_resolutions menu is redundant,
			// I'm leaving it out for now.
			//_gen_common_resolutions(xml);
			_fb_connectors.for_each([&] (Fb_connector const &connector) {
				if (connector.connected)
					_gen_connector(xml, connector);
			});
		});
	});
}


/*void Sculpt::Framebuffer_dialog::hover(Xml_node hover)
{
	bool const changed =
		_connector_item.match(hover, "vbox", "hbox", "button", "name") |
		_mode_item     .match(hover, "vbox", "frame", "vbox", "hbox", "name") |
		_connect_item  .match(hover, "vbox", "frame", "vbox", "button", "name");

	if (changed)
		_dialog_generator.generate_dialog();
}*/

void Sculpt::Framebuffer_dialog::hover(Xml_node hover)
{
	bool const changed =
		_connector_item.match(hover, "vbox", "button", "name") |
		_mode_item     .match(hover, "vbox", "frame", "vbox", "hbox", "name");

	if (changed)
		_dialog_generator.generate_dialog();
}

void Sculpt::Framebuffer_dialog::click(Action &action)
{
	_connector_item.toggle_selection_on_click();

	unsigned int connector_connected_count = 0;
	unsigned int connector_enabled_count = 0;

	_fb_connectors.for_each([&] (Fb_connector const& connector) {
		if (connector.connected) connector_connected_count++;
		if (connector.enabled) connector_enabled_count++;
	});

	// If there is only 1 connected connector, you can't change its resolution.
	// Just set it to >= 1 or remove the if condition in case you do want that ability.
	if (connector_connected_count >= 2) {
		_fb_connectors.for_each([&] (Fb_connector &connector) {
			if (_connector_item.selected(connector.name)) {
				if (connector.enabled && _mode_item.hovered(connector.selected_mode().id())) {
					if (connector_enabled_count >= 2) {
						connector.set_enabled(false);
						connector.select_mode("");

						// We don't want to use the selectable_item's selection at all:
						//_mode_item.toggle_selection_on_click();

						action.refresh_after_toggle();
						//_generate_fb_drv_config();
					}
				}

				else {
					connector.modes.for_each([&] (Fb_connector_mode const& mode) {
						if (_mode_item.hovered(mode.id())) {
							connector.set_enabled(true);
							connector.select_mode(mode.id());
							action.refresh_after_toggle();
						}
					});
				}
			}
		});
	}


/*
	if (_wifi_connection.connected() && _ap_item.hovered(_wifi_connection.bssid)) {
		action.wifi_disconnect();
		_ap_item.reset();
	} else {
		_ap_item.toggle_selection_on_click();

		immediately connect to unprotected access point when selected
		if (_ap_item.any_selected() && _selected_ap_unprotected())
			action.wifi_connect(selected_ap());
	}
	*/
}


void Sculpt::Framebuffer_dialog::clack(Action &/* action */)
{
	_dialog_generator.generate_dialog();
}

