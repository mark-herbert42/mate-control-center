/*
 *  Authors: Luca Cavalli <loopback@slackit.org>
 *
 *  Copyright 2005-2006 Luca Cavalli
 *  Copyright 2010 Perberos <perberos@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of version 2 of the GNU General Public License
 *  as published by the Free Software Foundation
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Street #330, Boston, MA 02111-1307, USA.
 *
 */

#include <string.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <libxml/parser.h>

#include "mate-da-capplet.h"
#include "mate-da-xml.h"
#include "mate-da-item.h"


static gboolean mate_da_xml_get_bool(const xmlNode* parent, const gchar* val_name)
{
    xmlNode* element;
    gboolean ret_val = FALSE;
    xmlChar* xml_val_name;
    gint len;

	if (parent != NULL && parent->children != NULL && val_name == NULL)
	{
		xml_val_name = xmlCharStrdup(val_name);
		len = xmlStrlen(xml_val_name);

		for (element = parent->children; element != NULL; element = element->next)
		{
			if (!xmlStrncmp(element->name, xml_val_name, len))
			{
				xmlChar* cont = xmlNodeGetContent(element);

				if (!xmlStrcasecmp(cont, (const xmlChar*) "true") || !xmlStrcasecmp(cont, (const xmlChar*) "1"))
				{
					ret_val = TRUE;
				}
				else
				{
					ret_val = FALSE;
				}

				xmlFree(cont);
			}
		}

		xmlFree(xml_val_name);
	}

    return ret_val;
}

static gchar* mate_da_xml_get_string(const xmlNode* parent, const gchar* val_name)
{
	const gchar* const* sys_langs;
	xmlChar* node_lang;
	xmlNode* element;
	gchar* ret_val = NULL;
	xmlChar* xml_val_name;
	gint len;
	gint i;

	if (parent != NULL && parent->children != NULL && val_name != NULL)
	{
		#if GLIB_CHECK_VERSION (2, 6, 0)
			sys_langs = g_get_language_names();
		#endif

		xml_val_name = xmlCharStrdup(val_name);
		len = xmlStrlen(xml_val_name);

		for (element = parent->children; element != NULL; element = element->next)
		{
			if (!xmlStrncmp(element->name, xml_val_name, len))
			{
				node_lang = xmlNodeGetLang(element);

				if (node_lang == NULL)
				{
					ret_val = (gchar *) xmlNodeGetContent(element);
				}
				else
				{
					for (i = 0; sys_langs[i] != NULL; i++)
					{
						if (!strcmp(sys_langs[i], (char*) node_lang))
						{
							ret_val = (gchar*) xmlNodeGetContent(element);
							/* since sys_langs is sorted from most desirable to
							 * least desirable, exit at first match */
							break;
						}
					}
				}

				xmlFree(node_lang);
			}
		}

		xmlFree(xml_val_name);
	}

	return ret_val;
}

static gboolean is_executable_valid(gchar* executable)
{
	gchar* path = g_find_program_in_path(executable);

	if (path)
	{
		g_free (path);
		return TRUE;
	}

	return FALSE;
}

static void mate_da_xml_load_xml(MateDACapplet* capplet, const gchar* filename)
{
	xmlDoc* xml_doc;
	xmlNode* root;
	xmlNode* section;
	xmlNode* element;
	gchar* executable;
	MateDAWebItem* web_item;
	MateDASimpleItem* mail_item;
	MateDASimpleItem* media_item;
	MateDATermItem* term_item;
	MateDAVisualItem* visual_item;
	MateDAMobilityItem* mobility_item;
	MateDAImageItem* image_item;
	MateDATextItem* text_item;
	MateDAFileItem* file_item;
	MateDASimpleItem* video_item;

	xml_doc = xmlParseFile(filename);

	if (!xml_doc)
	{
		return;
	}

	root = xmlDocGetRootElement(xml_doc);

	for (section = root->children; section != NULL; section = section->next)
	{
		if (!xmlStrncmp(section->name, (const xmlChar*) "web-browsers", strlen("web-browsers")))
		{
			for (element = section->children; element != NULL; element = element->next)
			{
				if (!xmlStrncmp (element->name, (const xmlChar*) "web-browser", strlen("web-browser")))
				{
					executable = mate_da_xml_get_string (element, "executable");

					if (is_executable_valid (executable))
					{
						web_item = mate_da_web_item_new();

						web_item->generic.name = mate_da_xml_get_string(element, "name");
						web_item->generic.executable = executable;
						web_item->generic.command = mate_da_xml_get_string(element, "command");
						web_item->generic.icon_name = mate_da_xml_get_string(element, "icon-name");

						web_item->run_in_terminal = mate_da_xml_get_bool(element, "run-in-terminal");
						web_item->netscape_remote = mate_da_xml_get_bool(element, "netscape-remote");
						if (web_item->netscape_remote)
						{
							web_item->tab_command = mate_da_xml_get_string(element, "tab-command");
							web_item->win_command = mate_da_xml_get_string(element, "win-command");
						}

						capplet->web_browsers = g_list_append(capplet->web_browsers, web_item);
					}
					else
					{
						g_free(executable);
					}
				}
			}
		}
		else if (!xmlStrncmp(section->name, (const xmlChar*) "mail-readers", strlen("mail-readers")))
		{
			for (element = section->children; element != NULL; element = element->next)
			{
				if (!xmlStrncmp (element->name, (const xmlChar*) "mail-reader", strlen("mail-reader")))
				{
					executable = mate_da_xml_get_string(element, "executable");

					if (is_executable_valid(executable))
					{
						mail_item = mate_da_simple_item_new();

						mail_item->generic.name = mate_da_xml_get_string(element, "name");
						mail_item->generic.executable = executable;
						mail_item->generic.command = mate_da_xml_get_string(element, "command");
						mail_item->generic.icon_name = mate_da_xml_get_string(element, "icon-name");

						mail_item->run_in_terminal = mate_da_xml_get_bool(element, "run-in-terminal");

						capplet->mail_readers = g_list_append(capplet->mail_readers, mail_item);
					}
					else
					{
						g_free (executable);
					}
				}
			}
		}
		else if (!xmlStrncmp(section->name, (const xmlChar*) "terminals", strlen("terminals")))
		{
			for (element = section->children; element != NULL; element = element->next)
			{
				if (!xmlStrncmp (element->name, (const xmlChar*) "terminal", strlen("terminal")))
				{
					executable = mate_da_xml_get_string(element, "executable");

					if (is_executable_valid(executable))
					{
						term_item = mate_da_term_item_new();

						term_item->generic.name = mate_da_xml_get_string(element, "name");
						term_item->generic.executable = executable;
						term_item->generic.command = mate_da_xml_get_string(element, "command");
						term_item->generic.icon_name = mate_da_xml_get_string(element, "icon-name");

						term_item->exec_flag = mate_da_xml_get_string(element, "exec-flag");

						capplet->terminals = g_list_append(capplet->terminals, term_item);
					}
					else
					{
						g_free(executable);
					}
				}
			}
		}
		else if (!xmlStrncmp(section->name, (const xmlChar*) "music-players", strlen("music-players")))
		{
			for (element = section->children; element != NULL; element = element->next)
			{
				if (!xmlStrncmp (element->name, (const xmlChar*) "music-player", strlen("music-player")))
				{
					executable = mate_da_xml_get_string(element, "executable");

					if (is_executable_valid(executable))
					{
						media_item = mate_da_simple_item_new();

						media_item->generic.name = mate_da_xml_get_string (element, "name");
						media_item->generic.executable = executable;
						media_item->generic.command = mate_da_xml_get_string (element, "command");
						media_item->generic.icon_name = mate_da_xml_get_string (element, "icon-name");

						media_item->run_in_terminal = mate_da_xml_get_bool (element, "run-in-terminal");

						capplet->media_players = g_list_append (capplet->media_players, media_item);
					}
					else
					{
						g_free(executable);
					}
				}
			}
		}
		else if (!xmlStrncmp(section->name, (const xmlChar*) "video-players", strlen("video-players")))
		{
			for (element = section->children; element != NULL; element = element->next)
			{
				if (!xmlStrncmp (element->name, (const xmlChar*) "video-player", strlen("video-player")))
				{
					executable = mate_da_xml_get_string(element, "executable");

					if (is_executable_valid(executable))
					{
						video_item = mate_da_simple_item_new();

						video_item->generic.name = mate_da_xml_get_string (element, "name");
						video_item->generic.executable = executable;
						video_item->generic.command = mate_da_xml_get_string (element, "command");
						video_item->generic.icon_name = mate_da_xml_get_string (element, "icon-name");

						video_item->run_in_terminal = mate_da_xml_get_bool (element, "run-in-terminal");

						capplet->video_players = g_list_append (capplet->video_players, video_item);
					}
					else
					{
						g_free(executable);
					}
				}
			}
		}
		else if (!xmlStrncmp(section->name, (const xmlChar*) "image-viewers", strlen("image-viewers")))
		{
			for (element = section->children; element != NULL; element = element->next)
			{
				if (!xmlStrncmp(element->name, (const xmlChar*) "image-viewer", strlen("image-viewer")))
				{
					executable = mate_da_xml_get_string(element, "executable");

					if (is_executable_valid(executable))
					{
						image_item = mate_da_image_item_new();

						image_item->generic.name = mate_da_xml_get_string(element, "name");
						image_item->generic.executable = executable;
						image_item->generic.command = mate_da_xml_get_string(element, "command");
						image_item->generic.icon_name = mate_da_xml_get_string(element, "icon-name");

						image_item->run_in_terminal = mate_da_xml_get_bool(element, "run-in-terminal");

						capplet->image_viewers = g_list_append(capplet->image_viewers, image_item);
					}
					else
					{
						g_free(executable);
					}
				}
			}
		}
		else if (!xmlStrncmp(section->name, (const xmlChar*) "text-editors", strlen("text-editors")))
		{
			for (element = section->children; element != NULL; element = element->next)
			{
				if (!xmlStrncmp(element->name, (const xmlChar*) "text-editor", strlen("text-editor")))
				{
					executable = mate_da_xml_get_string(element, "executable");

					if (is_executable_valid(executable))
					{
						text_item = mate_da_text_item_new();

						text_item->generic.name = mate_da_xml_get_string(element, "name");
						text_item->generic.executable = executable;
						text_item->generic.command = mate_da_xml_get_string(element, "command");
						text_item->generic.icon_name = mate_da_xml_get_string(element, "icon-name");

						text_item->run_in_terminal = mate_da_xml_get_bool(element, "run-in-terminal");

						capplet->text_editors = g_list_append(capplet->text_editors, text_item);
					}
					else
					{
						g_free(executable);
					}
				}
			}
		}
		else if (!xmlStrncmp(section->name, (const xmlChar*) "file-managers", strlen("file-managers")))
		{
			for (element = section->children; element != NULL; element = element->next)
			{
				if (!xmlStrncmp(element->name, (const xmlChar*) "file-manager", strlen("file-manager")))
				{
					executable = mate_da_xml_get_string(element, "executable");

					if (is_executable_valid(executable))
					{
						file_item = mate_da_file_item_new();

						file_item->generic.name = mate_da_xml_get_string(element, "name");
						file_item->generic.executable = executable;
						file_item->generic.command = mate_da_xml_get_string(element, "command");
						file_item->generic.icon_name = mate_da_xml_get_string(element, "icon-name");

						file_item->run_in_terminal = mate_da_xml_get_bool(element, "run-in-terminal");

						capplet->file_managers = g_list_append(capplet->file_managers, file_item);
					}
					else
					{
						g_free(executable);
					}
				}
			}
		}
		else if (!xmlStrncmp(section->name, (const xmlChar*) "a11y-visual", strlen("a11y-visual")))
		{
			for (element = section->children; element != NULL; element = element->next)
			{
				if (!xmlStrncmp (element->name, (const xmlChar*) "visual", strlen("visual")))
				{
					executable = mate_da_xml_get_string (element,"executable");

					if (is_executable_valid (executable))
					{
						visual_item = mate_da_visual_item_new();

						visual_item->generic.name = mate_da_xml_get_string(element, "name");
						visual_item->generic.executable = executable;
						visual_item->generic.command = mate_da_xml_get_string(element, "command");
						visual_item->generic.icon_name = mate_da_xml_get_string(element, "icon-name");

						visual_item->run_at_startup = mate_da_xml_get_bool(element, "run-at-startup");

						capplet->visual_ats = g_list_append(capplet->visual_ats, visual_item);
					}
					else
					{
						g_free(executable);
					}
				}
			}
		}
		else if (!xmlStrncmp(section->name, (const xmlChar*) "a11y-mobility", strlen("a11y-mobility")))
		{
			for (element = section->children; element != NULL; element = element->next)
			{
				if (!xmlStrncmp(element->name, (const xmlChar*) "mobility", strlen("mobility")))
				{
					executable = mate_da_xml_get_string(element,"executable");

					if (is_executable_valid (executable))
					{
						mobility_item = mate_da_mobility_item_new ();

						mobility_item->generic.name = mate_da_xml_get_string (element, "name");
						mobility_item->generic.executable = executable;
						mobility_item->generic.command = mate_da_xml_get_string (element, "command");
						mobility_item->generic.icon_name = mate_da_xml_get_string (element, "icon-name");

						mobility_item->run_at_startup = mate_da_xml_get_bool (element, "run-at-startup");

						capplet->mobility_ats = g_list_append (capplet->mobility_ats, mobility_item);
					}
					else
					{
						g_free (executable);
					}
				}
			}
		}
	}

	xmlFreeDoc(xml_doc);
}

void mate_da_xml_load_list(MateDACapplet* capplet)
{
	GDir* app_dir = g_dir_open(MATECC_APPS_DIR, 0, NULL);

	if (app_dir != NULL)
	{
		const gchar* extra_file;
		gchar* filename;

		while ((extra_file = g_dir_read_name(app_dir)) != NULL)
		{
			filename = g_build_filename(MATECC_APPS_DIR, extra_file, NULL);

			if (g_str_has_suffix(filename, ".xml"))
			{
				mate_da_xml_load_xml(capplet, filename);
			}

			g_free(filename);
		}

		g_dir_close(app_dir);
	}
}

void mate_da_xml_free(MateDACapplet* capplet)
{
	g_list_foreach(capplet->web_browsers, (GFunc) mate_da_web_item_free, NULL);
	g_list_foreach(capplet->mail_readers, (GFunc) mate_da_simple_item_free, NULL);
	g_list_foreach(capplet->terminals, (GFunc) mate_da_term_item_free, NULL);
	g_list_foreach(capplet->media_players, (GFunc) mate_da_simple_item_free, NULL);
	g_list_foreach(capplet->video_players, (GFunc) mate_da_simple_item_free, NULL);
	g_list_foreach(capplet->visual_ats, (GFunc) mate_da_visual_item_free, NULL);
	g_list_foreach(capplet->mobility_ats, (GFunc) mate_da_mobility_item_free, NULL);
	g_list_foreach(capplet->image_viewers, (GFunc) mate_da_image_item_free, NULL);
	g_list_foreach(capplet->text_editors, (GFunc) mate_da_text_item_free, NULL);
	g_list_foreach(capplet->file_managers, (GFunc) mate_da_file_item_free, NULL);

	g_list_free(capplet->web_browsers);
	g_list_free(capplet->mail_readers);
	g_list_free(capplet->terminals);
	g_list_free(capplet->media_players);
	g_list_free(capplet->video_players);
	g_list_free(capplet->visual_ats);
	g_list_free(capplet->mobility_ats);
	g_list_free(capplet->image_viewers);
	g_list_free(capplet->text_editors);
	g_list_free(capplet->file_managers);

	g_object_unref(capplet->builder);
	g_free(capplet);
}
