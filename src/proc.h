/***************************************************************************
 *            proc.h
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

#ifndef __PROC_H__
#	define __PROC_H__
#	include <fftw3.h>
#	include "main.h"
	extern GtkWidget *fst, *jind, *jind2, *kind, *notebook, *notebook2, *statusbar, *plot1, *plot2, *plot3, *pr, *window, *zpd;
	extern GtkWidget *chi, *chil, *dBs, *dsl, *lcmp, *neg, *oft, *opttri, *rest, *trans, *twopionx, *visl, *wll;
	extern GArray *chp, *doms, *vis;
	extern GArray *bsra, *bspa, *isra, *ispa, *tca, *twa, *zwa;
	extern GSList *group2;
	extern guint flags, flagd, kdimxf;
	extern gdouble oe;
	void prs(GtkWidget*, gpointer);
	void trs(GtkWidget*, gpointer);
#endif
