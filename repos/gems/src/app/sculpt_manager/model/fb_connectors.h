/*
 * \brief  Representation of a framebuffer connector
 * \author Roman Iten
 * \date   2018-08-18
 */

#ifndef _MODEL__FB_CONNECTOR_H_
#define _MODEL__FB_CONNECTOR_H_

#include "types.h"
#include "fb_connector_mode.h"

namespace Sculpt {

	struct Fb_connector;
	struct Fb_connector_update_policy;

	typedef List_model<Fb_connector> Fb_connectors;
};


struct Sculpt::Fb_connector : List_model<Fb_connector>::Element
{
	typedef String<32> Name;
	typedef String<16> Id;

	Name const               name;
	bool                     connected;
	bool                     enabled;
	Fb_connector_modes       modes { };
	Fb_connector_mode::Id    selected_mode_id {""};

	Fb_connector_mode selected_mode() const {
		Fb_connector_mode returned_mode;
		
		unsigned max_width = 0;
		unsigned max_height = 0;

		if (selected_mode_id == "") {
			modes.for_each([&] (Fb_connector_mode const& mode) {
				if ((mode.width*mode.height) > (max_width*max_height)) {
					max_width = mode.width;
					max_height = mode.height;
					returned_mode = mode;
				}
			});
		}

		else {
			modes.for_each([&] (Fb_connector_mode const& mode) {
				if (mode.id() == selected_mode_id)
					returned_mode = mode;
			});
		}

		return returned_mode;
	}

	/*bool common(Fb_connectors const& connectors) const {}*/

	void toggle() {
		enabled = !enabled;
	}

	void set_enabled(bool b) {
		enabled = b;
	}

	void select_mode(Id m) {
		selected_mode_id = m;
	}

	Fb_connector(Name const &name, bool connected/* , bool enabled */)
	: name(name), connected(connected), enabled(true) { }
};


/**
 * Policy for transforming a 'connectors' report into a list model
 */
struct Sculpt::Fb_connector_update_policy : List_model<Fb_connector>::Update_policy
{
	Allocator& _alloc;
	Fb_connectors& _connectors;

	Fb_connector_update_policy(Allocator &alloc, Fb_connectors& connectors)
	: _alloc(alloc), _connectors(connectors) { }

	void destroy_element(Fb_connector &elem)
	{
		log("DESTROYING CONNECTOR");
		destroy(_alloc, &elem);
	}

	Fb_connector &create_element(Xml_node const& node)
	{
		Fb_connector& connector = *new (_alloc)
			Fb_connector(node.attribute_value("name", Fb_connector::Name()),
			             node.attribute_value("connected", false));

		Fb_connector_mode_update_policy policy(_alloc/*, _connectors*/);
		connector.modes.update_from_xml(policy, node);

		return connector;
	}

	void update_element(Fb_connector& connector, const Xml_node& node)
	{
		connector.connected = node.attribute_value("connected", false);

		Fb_connector_mode_update_policy policy(_alloc/*, _connectors*/);
		connector.modes.update_from_xml(policy, node);
	}

	static bool element_matches_xml_node(Fb_connector const& elem, Xml_node const& node)
	{
		return node.attribute_value("name", Fb_connector::Name()) == elem.name;
	}
};


#endif /* _MODEL__FB_CONNECTOR_H_ */
