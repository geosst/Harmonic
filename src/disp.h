/***************************************************************************
 *            disp.h
 *
 *  Sat Dec  4 17:18:14 2010
 *  Copyright  2010  Paul Childs
 *  <pchilds@physics.org>
 ****************************************************************************/

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */

#ifndef __DISP_H__
#	define __DISP_H__
#	include "main.h"
	extern GtkWidget *chi, *jind, *kind, *notebook2, *plot1, *plot2, *plot3, *window;
	extern GArray *bsra, *chp, *doms, *isra, *vis;
	extern guint flagd, flags, kdimxf;
	extern void upj(GtkWidget*, gpointer);
	extern void upk(GtkWidget*, gpointer);
	void dpa(GtkWidget*, gpointer);
	void dpo(GtkWidget*, gpointer);
	void upc2(GtkWidget*, gpointer);
	void upc3(GtkWidget*, gpointer);
	void upc4(GtkWidget*, gpointer);
	void upj3(GtkWidget*, gpointer);
	void upj4(GtkWidget*, gpointer);
	void upk2(GtkWidget*, gpointer);
	void dpr(GtkWidget*, gpointer);
#endif
