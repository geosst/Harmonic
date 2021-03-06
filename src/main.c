/***************************************************************************
 *            main.c
 *
 *  A GTK+ program for analysing data in the Fourier domain
 *  version 0.1.0
 *  Features:
 *            Multiple data format input
 *            Graphical display of data and transformed result
 *            Batch Processing
 *            Analysis for visibility, domain shifts and chirp
 *            Publication ready graphs of results in multiple formats
 *
 *  Sat Dec  4 17:18:14 2010
 *  Copyright  2011  Paul Childs
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

/*
 * TO DO:
 *
 * BAT: skip over erroneous data points (maybe duplicate previous/next?)
 * FFT: implement invert to 2pi/x routine
 * PRC: triangle optimisation
 * SAV: session save/restore routine
 * TRS: wavelets
 * PLOT: see issues in gtkplotpolar
 */

#include <gdk/gdkkeysyms.h>
#include <fftw3.h>
#include "main.h"
#include "batc.h"
#include "disp.h"
#include "data.h"
#include "proc.h"
#include "util.h"

GArray *bspa, *bsra, *chp, *doms, *ispa, *isra, *tca, *twa, *vis, *zwa; /* arrays for windowing and data */
gchar *fold=NULL, *folr=NULL;
gdouble oe=0; /* value to hold prior reference level for offset tracking */
GSList *group=NULL, *group2=NULL, *group3=NULL, *group4=NULL, *group5=NULL;
GtkWidget *chil, *dsl, *fst, *notebook, *notebook2, *plot1, *plot2, *plot3, *pr, *rest, *statusbar, *tr, *trac, *tracmenu, *visl, *window, *zpd;
GtkWidget *agosa, *agtl, *anosa, *chi, *db4, *db8, *dBs, *dlm, *frr, *lcmp, *mg, *mgp, *mrl, *myr, *ncmp, *neg, *oft, *opttri, *ri, *sws, *trans, *twopionx, *wll;
GtkWidget *bsp, *bsr, *isp, *isr, *jind, *jind2, *kind, *tc, *tw, *zw; /* widgets for windowing */
guint flagd=0, flags=0, kdimxf=1; /* display flags, current processing state and number of kdim's in batch process */
gulong j1_id, j2_id, k_id, bsr_id, bsp_id, isr_id, isp_id, tc_id, tw_id, zw_id; /* id for disabling/enabling post-transform processing */

int main(int argc, char *argv[])
{
	AtkObject *atk_label, *atk_widget;
	GArray *caa, *cab, *cag, *car;
	gdouble fll=0;
	GtkAccelGroup *accel_group=NULL;
	GtkAdjustment *adj;
	GtkWidget *butt, *hpane, *label, *mnb, *mni, *mnu, *smnu, *table, *vbox;

	bindtextdomain(PACKAGE, LOCALEDIR);
	bind_textdomain_codeset(PACKAGE, "UTF-8");
	textdomain(PACKAGE);
	gtk_init(&argc, &argv);
	window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), _("Harmonic Spectrum Analyser"));
	g_signal_connect_swapped(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);
	accel_group=gtk_accel_group_new();
	gtk_window_add_accel_group(GTK_WINDOW(window), accel_group);
	vbox=gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window), vbox);
	gtk_widget_show(vbox);
	mnb=gtk_menu_bar_new();
	gtk_box_pack_start(GTK_BOX(vbox), mnb, FALSE, FALSE, 2);
	gtk_widget_show(mnb);
	mnu=gtk_menu_new();
	smnu=gtk_menu_new();
	mni=gtk_menu_item_new_with_label(_("Data"));
	gtk_widget_add_accelerator(mni, "activate", accel_group, GDK_o, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	g_signal_connect(G_OBJECT(mni), "activate", G_CALLBACK(opd), NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(smnu), mni);
	gtk_widget_show(mni);
	mni=gtk_menu_item_new_with_label(_("Batch File"));
	gtk_widget_add_accelerator(mni, "activate", accel_group, GDK_b, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	g_signal_connect(G_OBJECT(mni), "activate", G_CALLBACK(bat), NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(smnu), mni);
	gtk_widget_show(mni);
	mni=gtk_image_menu_item_new_from_stock(GTK_STOCK_OPEN, NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(mnu), mni);
	gtk_widget_show(mni);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(mni), smnu);
	smnu=gtk_menu_new();
	mni=gtk_menu_item_new_with_label(_("Data"));
	gtk_widget_add_accelerator(mni, "activate", accel_group, GDK_s, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	g_signal_connect(G_OBJECT(mni), "activate", G_CALLBACK(sav), NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(smnu), mni);
	gtk_widget_show(mni);
	mni=gtk_menu_item_new_with_label(_("Graph"));
	gtk_widget_add_accelerator(mni, "activate", accel_group, GDK_g, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	g_signal_connect(G_OBJECT(mni), "activate", G_CALLBACK(prg), NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(smnu), mni);
	gtk_widget_show(mni);
	mni=gtk_image_menu_item_new_from_stock(GTK_STOCK_SAVE, NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(mnu), mni);
	gtk_widget_show(mni);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(mni), smnu);
	mni=gtk_image_menu_item_new_from_stock(GTK_STOCK_PRINT, NULL);
	gtk_widget_add_accelerator(mni, "activate", accel_group, GDK_p, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	g_signal_connect(G_OBJECT(mni), "activate", G_CALLBACK(prt), (gpointer)window);
	gtk_menu_shell_append(GTK_MENU_SHELL(mnu), mni);
	gtk_widget_show(mni);
	mni=gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(mnu), mni);
	gtk_widget_show(mni);
	mni=gtk_menu_item_new_with_label(_("Restore Session"));
	g_signal_connect(G_OBJECT(mni), "activate", G_CALLBACK(sessres), NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(mnu), mni);
	gtk_widget_show(mni);
	mni=gtk_menu_item_new_with_label(_("Save Session"));
	g_signal_connect(G_OBJECT(mni), "activate", G_CALLBACK(sesssav), NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(mnu), mni);
	gtk_widget_show(mni);
	mni=gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(mnu), mni);
	gtk_widget_show(mni);
	mni=gtk_image_menu_item_new_from_stock(GTK_STOCK_QUIT, accel_group);
	gtk_widget_add_accelerator(mni, "activate", accel_group, GDK_q, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	g_signal_connect(G_OBJECT(mni), "activate", G_CALLBACK(gtk_main_quit), NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(mnu), mni);
	gtk_widget_show(mni);
	mni=gtk_menu_item_new_with_mnemonic(_("_File"));
	gtk_widget_show(mni);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(mni), mnu);
	gtk_menu_shell_append(GTK_MENU_SHELL(mnb), mni);
	mnu=gtk_menu_new();
	smnu=gtk_menu_new();
	agosa=gtk_radio_menu_item_new_with_label(group, "Agilent OSA");
	group=gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(agosa));
	gtk_menu_shell_append(GTK_MENU_SHELL(smnu), agosa);
	gtk_widget_show(agosa);
	agtl=gtk_radio_menu_item_new_with_label(group, "Agilent Tunable Laser");
	group=gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(agtl));
	gtk_menu_shell_append(GTK_MENU_SHELL(smnu), agtl);
	gtk_widget_show(agtl);
	anosa=gtk_radio_menu_item_new_with_label(group, "Ando OSA");
	group=gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(anosa));
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(anosa), TRUE);
	gtk_menu_shell_append(GTK_MENU_SHELL(smnu), anosa);
	gtk_widget_show(anosa);
	sws=gtk_radio_menu_item_new_with_label(group, "JDSUniphase Swept Wavelength System");
	group=gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(sws));
	gtk_menu_shell_append(GTK_MENU_SHELL(smnu), sws);
	gtk_widget_show(sws);
	dlm=gtk_radio_menu_item_new_with_label(group, _("Raw Delimited Data"));
	group=gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(dlm));
	gtk_menu_shell_append(GTK_MENU_SHELL(smnu), dlm);
	gtk_widget_show(dlm);
	mni=gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(smnu), mni);
	gtk_widget_show(mni);
	mg=gtk_radio_menu_item_new_with_label(group5, _("Magnitude Only"));
	group5=gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(mg));
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(mg), TRUE);
	gtk_menu_shell_append(GTK_MENU_SHELL(smnu), mg);
	gtk_widget_show(mg);
	mgp=gtk_radio_menu_item_new_with_label(group5, _("Magnitude and Phase"));
	group5=gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(mgp));
	gtk_menu_shell_append(GTK_MENU_SHELL(smnu), mgp);
	gtk_widget_show(mgp);
	ri=gtk_radio_menu_item_new_with_label(group5, _("Real/Imaginary"));
	group5=gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(ri));
	gtk_menu_shell_append(GTK_MENU_SHELL(smnu), ri);
	gtk_widget_show(ri);
	mni=gtk_menu_item_new_with_label(_("Data Format:"));
	gtk_menu_shell_append(GTK_MENU_SHELL(mnu), mni);
	gtk_widget_show(mni);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(mni), smnu);
	tracmenu=gtk_menu_new();
	trac=gtk_menu_item_new_with_label(_("Trace:"));
	gtk_menu_shell_append(GTK_MENU_SHELL(mnu), trac);
	gtk_widget_show(trac);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(trac), tracmenu);
	mni=gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(mnu), mni);
	gtk_widget_show(mni);
	trans=gtk_check_menu_item_new_with_label(_("Transmission Measurement?"));
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(trans), TRUE);
	gtk_menu_shell_append(GTK_MENU_SHELL(mnu), trans);
	gtk_widget_show(trans);
	dBs=gtk_check_menu_item_new_with_label(_("Data in dBs?"));
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(dBs), TRUE);
	gtk_menu_shell_append(GTK_MENU_SHELL(mnu), dBs);
	gtk_widget_show(dBs);
	neg=gtk_check_menu_item_new_with_label(_("Negate?"));
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(neg), FALSE);
	gtk_menu_shell_append(GTK_MENU_SHELL(mnu), neg);
	gtk_widget_show(neg);
	mni=gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(mnu), mni);
	gtk_widget_show(mni);
	mni=gtk_menu_item_new_with_label(_("Display Properties:"));
	gtk_widget_add_accelerator(mni, "activate", accel_group, GDK_F2, 0, GTK_ACCEL_VISIBLE);
	g_signal_connect(G_OBJECT(mni), "activate", G_CALLBACK(dpr), NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(mnu), mni);
	gtk_widget_show(mni);
	mni=gtk_menu_item_new_with_mnemonic(_("_Properties"));
	gtk_widget_show(mni);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(mni), mnu);
	gtk_menu_shell_append(GTK_MENU_SHELL(mnb), mni);
	mnu=gtk_menu_new();
	smnu=gtk_menu_new();
	ncmp=gtk_radio_menu_item_new_with_label(group3, _("None"));
	group3=gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(ncmp));
	gtk_menu_shell_append(GTK_MENU_SHELL(smnu), ncmp);
	gtk_widget_show(ncmp);
	lcmp=gtk_radio_menu_item_new_with_label(group3, _("1st order\nspectral shadowing"));
	group3=gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(lcmp));
	gtk_menu_shell_append(GTK_MENU_SHELL(smnu), lcmp);
	gtk_widget_show(lcmp);
	mni=gtk_menu_item_new_with_label(_("Nonlinear\nCompensation:"));
	gtk_menu_shell_append(GTK_MENU_SHELL(mnu), mni);
	gtk_widget_show(mni);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(mni), smnu);
	smnu=gtk_menu_new();
	frr=gtk_radio_menu_item_new_with_label(group4, _("Fourier"));
	group4=gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(frr));
	gtk_menu_shell_append(GTK_MENU_SHELL(smnu), frr);
	gtk_widget_show(frr);
	db4=gtk_radio_menu_item_new_with_label(group4, _("Daubechies 4"));
	group4=gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(db4));
	gtk_menu_shell_append(GTK_MENU_SHELL(smnu), db4);
	gtk_widget_show(db4);
	db8=gtk_radio_menu_item_new_with_label(group4, _("Daubechies 8"));
	group4=gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(db8));
	gtk_menu_shell_append(GTK_MENU_SHELL(smnu), db8);
	gtk_widget_show(db8);
	myr=gtk_radio_menu_item_new_with_label(group4, _("Meyer"));
	group4=gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(myr));
	gtk_menu_shell_append(GTK_MENU_SHELL(smnu), myr);
	gtk_widget_show(myr);
	mrl=gtk_radio_menu_item_new_with_label(group4, _("Morlet"));
	group4=gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(mrl));
	gtk_menu_shell_append(GTK_MENU_SHELL(smnu), mrl);
	gtk_widget_show(mrl);
	mni=gtk_menu_item_new_with_label(_("Basis Function:"));
	gtk_menu_shell_append(GTK_MENU_SHELL(mnu), mni);
	gtk_widget_show(mni);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(mni), smnu);
	twopionx=gtk_check_menu_item_new_with_label(_("Invert domain?"));
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(twopionx), FALSE);
	gtk_menu_shell_append(GTK_MENU_SHELL(mnu), twopionx);
	gtk_widget_show(twopionx);
	wll=gtk_check_menu_item_new_with_label(_("Apply correction\nto Window edges?"));
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(wll), FALSE);
	gtk_menu_shell_append(GTK_MENU_SHELL(mnu), wll);
	gtk_widget_show(wll);
	mni=gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(mnu), mni);
	gtk_widget_show(mni);
	opttri=gtk_check_menu_item_new_with_label(_("Optimise Triangle fit?"));
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(opttri), FALSE);
	gtk_menu_shell_append(GTK_MENU_SHELL(mnu), opttri);
	gtk_widget_show(opttri);
	chi=gtk_check_menu_item_new_with_label(_("Calculate Chirp?"));
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(chi), FALSE);
	gtk_menu_shell_append(GTK_MENU_SHELL(mnu), chi);
	gtk_widget_show(chi);
	mni=gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(mnu), mni);
	gtk_widget_show(mni);
	oft=gtk_check_menu_item_new_with_label(_("Autotrack Offset?"));
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(oft), FALSE);
	gtk_menu_shell_append(GTK_MENU_SHELL(mnu), oft);
	gtk_widget_show(oft);
	mni=gtk_menu_item_new_with_mnemonic(_("_Advanced"));
	gtk_widget_show(mni);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(mni), mnu);
	gtk_menu_shell_append(GTK_MENU_SHELL(mnb), mni);
	mnu=gtk_menu_new();
#ifdef USE_GDOC
	mni=gtk_menu_item_new_with_label(_("Instructions"));
	gtk_widget_add_accelerator(mni, "activate", accel_group, GDK_F1, 0, GTK_ACCEL_VISIBLE);
	g_signal_connect(G_OBJECT(mni), "activate", G_CALLBACK(help), NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(mnu), mni);
	gtk_widget_show(mni);
#endif
	mni=gtk_image_menu_item_new_from_stock(GTK_STOCK_ABOUT, NULL);
	g_signal_connect(G_OBJECT(mni), "activate", G_CALLBACK(about), NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(mnu), mni);
	gtk_widget_show(mni);
	mni=gtk_menu_item_new_with_mnemonic(_("_Help"));
	gtk_widget_show(mni);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(mni), mnu);
	gtk_menu_shell_append(GTK_MENU_SHELL(mnb), mni);
	gtk_menu_item_right_justify(GTK_MENU_ITEM(mni));
	hpane=gtk_hpaned_new();
	gtk_widget_show(hpane);
	gtk_container_add(GTK_CONTAINER(vbox), hpane);
	{bsra=g_array_new(FALSE, FALSE, sizeof(gdouble)); bspa=g_array_new(FALSE, FALSE, sizeof(gdouble)); isra=g_array_new(FALSE, FALSE, sizeof(gdouble)); ispa=g_array_new(FALSE, FALSE, sizeof(gdouble)); tca=g_array_new(FALSE, FALSE, sizeof(gdouble)); twa=g_array_new(FALSE, FALSE, sizeof(gdouble)); zwa=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	g_array_append_val(bsra, fll);
	fll++;
	g_array_append_val(bspa, fll);
	g_array_append_val(isra, fll);
	fll++;
	g_array_append_val(tca, fll);
	g_array_append_val(twa, fll);
	g_array_append_val(zwa, fll);
	fll++;
	g_array_append_val(ispa, fll);
	notebook=gtk_notebook_new();
	gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook), GTK_POS_TOP);
	table=gtk_table_new(5, 3, FALSE);
	gtk_widget_show(table);
	label=gtk_label_new(_("Spectrum Start:"));
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(label);
	adj=(GtkAdjustment*) gtk_adjustment_new(0, -G_MAXDOUBLE, G_MAXDOUBLE, 1.0, 5.0, 0.0);
	bsr=gtk_spin_button_new(adj, 0.5, 3);
	bsr_id=g_signal_connect(G_OBJECT(bsr), "value-changed", G_CALLBACK(upa1), (gpointer) bsra);
	gtk_table_attach(GTK_TABLE(table), bsr, 0, 1, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(bsr);
	atk_widget=gtk_widget_get_accessible(bsr);
	atk_label=gtk_widget_get_accessible(GTK_WIDGET(label));
	atk_object_add_relationship(atk_label, ATK_RELATION_LABEL_FOR, atk_widget);
	atk_object_add_relationship(atk_widget, ATK_RELATION_LABELLED_BY, atk_label);
	label=gtk_label_new(_("Spectrum Stop:"));
	gtk_table_attach(GTK_TABLE(table), label, 1, 2, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(label);
	adj=(GtkAdjustment*) gtk_adjustment_new(1, -G_MAXDOUBLE, G_MAXDOUBLE, 1.0, 5.0, 0.0);
	bsp=gtk_spin_button_new(adj, 0.5, 3);
	bsp_id=g_signal_connect(G_OBJECT(bsp), "value-changed", G_CALLBACK(upa2), (gpointer) bspa);
	gtk_table_attach(GTK_TABLE(table), bsp, 1, 2, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(bsp);
	atk_widget=gtk_widget_get_accessible(bsp);
	atk_label=gtk_widget_get_accessible(GTK_WIDGET(label));
	atk_object_add_relationship(atk_label, ATK_RELATION_LABEL_FOR, atk_widget);
	atk_object_add_relationship(atk_widget, ATK_RELATION_LABELLED_BY, atk_label);
	label=gtk_label_new(_("Offset:"));
	gtk_table_attach(GTK_TABLE(table), label, 1, 2, 2, 3, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(label);
	adj=(GtkAdjustment*) gtk_adjustment_new(1, -G_MAXDOUBLE, G_MAXDOUBLE, 1.0, 5.0, 0.0);
	fst=gtk_spin_button_new(adj, 0.5, 2);
	gtk_table_attach(GTK_TABLE(table), fst, 1, 2, 3, 4, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(fst);
	atk_widget=gtk_widget_get_accessible(fst);
	atk_label=gtk_widget_get_accessible(GTK_WIDGET(label));
	atk_object_add_relationship(atk_label, ATK_RELATION_LABEL_FOR, atk_widget);
	atk_object_add_relationship(atk_widget, ATK_RELATION_LABELLED_BY, atk_label);
	label=gtk_label_new(_("Zero Padding 2^:"));
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, 2, 3, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(label);
	adj=(GtkAdjustment*) gtk_adjustment_new(12, 4, 31, 1.0, 5.0, 0.0);
	zpd=gtk_spin_button_new(adj, 0, 0);
	gtk_table_attach(GTK_TABLE(table), zpd, 0, 1, 3, 4, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(zpd);
	atk_widget=gtk_widget_get_accessible(zpd);
	atk_label=gtk_widget_get_accessible(GTK_WIDGET(label));
	atk_object_add_relationship(atk_label, ATK_RELATION_LABEL_FOR, atk_widget);
	atk_object_add_relationship(atk_widget, ATK_RELATION_LABELLED_BY, atk_label);
	label=gtk_label_new(_("j index:"));
	gtk_table_attach(GTK_TABLE(table), label, 2, 3, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(label);
	adj=(GtkAdjustment*) gtk_adjustment_new(0, 0, G_MAXINT8, 1.0, 5.0, 0.0);
	jind=gtk_spin_button_new(adj, 0, 0);
	j1_id=g_signal_connect(G_OBJECT(jind), "value-changed", G_CALLBACK(upj), NULL);
	gtk_table_attach(GTK_TABLE(table), jind, 2, 3, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(jind);
	atk_widget=gtk_widget_get_accessible(jind);
	atk_label=gtk_widget_get_accessible(GTK_WIDGET(label));
	atk_object_add_relationship(atk_label, ATK_RELATION_LABEL_FOR, atk_widget);
	atk_object_add_relationship(atk_widget, ATK_RELATION_LABELLED_BY, atk_label);
	butt=gtk_button_new_with_label(_("Reset\nArrays"));
	g_signal_connect(G_OBJECT(butt), "clicked", G_CALLBACK(reset), NULL);
	gtk_table_attach(GTK_TABLE(table), butt, 2, 3, 2, 4, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(butt);
	tr=gtk_button_new_with_label(_("Transform Spectrum"));
	g_signal_connect(G_OBJECT(tr), "clicked", G_CALLBACK(trs), NULL);
	gtk_table_attach(GTK_TABLE(table), tr, 0, 3, 4, 5, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(tr);
	label=gtk_label_new(_("Spectrum"));
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), table, label);
	table=gtk_table_new(6, 3, FALSE);
	gtk_widget_show(table);
	label=gtk_label_new(_("Inverse Spectrum Start:"));
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(label);
	adj=(GtkAdjustment*) gtk_adjustment_new(1, -G_MAXDOUBLE, G_MAXDOUBLE, 1.0, 5.0, 0.0);
	isr=gtk_spin_button_new(adj, 0.5, 3);
	isr_id=g_signal_connect(G_OBJECT(isr), "value-changed", G_CALLBACK(upa2), (gpointer) isra);
	gtk_table_attach(GTK_TABLE(table), isr, 0, 1, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(isr);
	atk_widget=gtk_widget_get_accessible(isr);
	atk_label=gtk_widget_get_accessible(GTK_WIDGET(label));
	atk_object_add_relationship(atk_label, ATK_RELATION_LABEL_FOR, atk_widget);
	atk_object_add_relationship(atk_widget, ATK_RELATION_LABELLED_BY, atk_label);
	label=gtk_label_new(_("Inverse Spectrum Stop:"));
	gtk_table_attach(GTK_TABLE(table), label, 1, 2, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(label);
	adj=(GtkAdjustment*) gtk_adjustment_new(3, -G_MAXDOUBLE, G_MAXDOUBLE, 1.0, 5.0, 0.0);
	isp=gtk_spin_button_new(adj, 0.5, 3);
	isp_id=g_signal_connect(G_OBJECT(isp), "value-changed", G_CALLBACK(upa2), (gpointer) ispa);
	gtk_table_attach(GTK_TABLE(table), isp, 1, 2, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(isp);
	atk_widget=gtk_widget_get_accessible(isp);
	atk_label=gtk_widget_get_accessible(GTK_WIDGET(label));
	atk_object_add_relationship(atk_label, ATK_RELATION_LABEL_FOR, atk_widget);
	atk_object_add_relationship(atk_widget, ATK_RELATION_LABELLED_BY, atk_label);
	label=gtk_label_new(_("Triangle Centre:"));
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, 2, 3, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(label);
	adj=(GtkAdjustment*) gtk_adjustment_new(2, -G_MAXDOUBLE, G_MAXDOUBLE, 1.0, 5.0, 0.0);
	tc=gtk_spin_button_new(adj, 0.5, 3);
	tc_id=g_signal_connect(G_OBJECT(tc), "value-changed", G_CALLBACK(upa2), (gpointer) tca);
	gtk_table_attach(GTK_TABLE(table), tc, 0, 1, 3, 4, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(tc);
	atk_widget=gtk_widget_get_accessible(tc);
	atk_label=gtk_widget_get_accessible(GTK_WIDGET(label));
	atk_object_add_relationship(atk_label, ATK_RELATION_LABEL_FOR, atk_widget);
	atk_object_add_relationship(atk_widget, ATK_RELATION_LABELLED_BY, atk_label);
	label=gtk_label_new(_("Triangle Full Width:"));
	gtk_table_attach(GTK_TABLE(table), label, 1, 2, 2, 3, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(label);
	adj=(GtkAdjustment*) gtk_adjustment_new(2, DZE, G_MAXDOUBLE, 1.0, 5.0, 0.0);
	tw=gtk_spin_button_new(adj, 0.5, 3);
	tw_id=g_signal_connect(G_OBJECT(tw), "value-changed", G_CALLBACK(upa2), (gpointer) twa);
	gtk_table_attach(GTK_TABLE(table), tw, 1, 2, 3, 4, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(tw);
	atk_widget=gtk_widget_get_accessible(tw);
	atk_label=gtk_widget_get_accessible(GTK_WIDGET(label));
	atk_object_add_relationship(atk_label, ATK_RELATION_LABEL_FOR, atk_widget);
	atk_object_add_relationship(atk_widget, ATK_RELATION_LABELLED_BY, atk_label);
	label=gtk_label_new(_("DC Peak Width:"));
	gtk_table_attach(GTK_TABLE(table), label, 1, 2, 4, 5, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(label);
	adj=(GtkAdjustment*) gtk_adjustment_new(2, DZE, G_MAXDOUBLE, 1.0, 5.0, 0.0);
	zw=gtk_spin_button_new(adj, 0.5, 3);
	zw_id=g_signal_connect(G_OBJECT(zw), "value-changed", G_CALLBACK(upa1), (gpointer) zwa);
	gtk_table_attach(GTK_TABLE(table), zw, 1, 2, 5, 6, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(zw);
	atk_widget=gtk_widget_get_accessible(zw);
	atk_label=gtk_widget_get_accessible(GTK_WIDGET(label));
	atk_object_add_relationship(atk_label, ATK_RELATION_LABEL_FOR, atk_widget);
	atk_object_add_relationship(atk_widget, ATK_RELATION_LABELLED_BY, atk_label);
	label=gtk_label_new(_("j index:"));
	gtk_table_attach(GTK_TABLE(table), label, 2, 3, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(label);
	adj=(GtkAdjustment*) gtk_adjustment_new(0, 0, G_MAXINT8, 1.0, 5.0, 0.0);
	jind2=gtk_spin_button_new(adj, 0, 0);
	j2_id=g_signal_connect(G_OBJECT(jind2), "value-changed", G_CALLBACK(upj), NULL);
	gtk_table_attach(GTK_TABLE(table), jind2, 2, 3, 1, 2, GTK_FILL|GTK_SHRINK, GTK_FILL|GTK_SHRINK, 2, 2);
	gtk_widget_show(jind2);
	atk_widget=gtk_widget_get_accessible(jind2);
	atk_label=gtk_widget_get_accessible(GTK_WIDGET(label));
	atk_object_add_relationship(atk_label, ATK_RELATION_LABEL_FOR, atk_widget);
	atk_object_add_relationship(atk_widget, ATK_RELATION_LABELLED_BY, atk_label);
	label=gtk_label_new(_("k index:"));
	gtk_table_attach(GTK_TABLE(table), label, 2, 3, 2, 3, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(label);
	adj=(GtkAdjustment*) gtk_adjustment_new(0, 0, G_MAXINT8, 1.0, 5.0, 0.0);
	kind=gtk_spin_button_new(adj, 0, 0);
	k_id=g_signal_connect(G_OBJECT(kind), "value-changed", G_CALLBACK(upk), NULL);
	gtk_table_attach(GTK_TABLE(table), kind, 2, 3, 3, 4, GTK_FILL|GTK_SHRINK, GTK_FILL|GTK_SHRINK, 2, 2);
	gtk_widget_show(kind);
	atk_widget=gtk_widget_get_accessible(kind);
	atk_label=gtk_widget_get_accessible(GTK_WIDGET(label));
	atk_object_add_relationship(atk_label, ATK_RELATION_LABEL_FOR, atk_widget);
	atk_object_add_relationship(atk_widget, ATK_RELATION_LABELLED_BY, atk_label);
	butt=gtk_button_new_with_label(_("Reset\nArrays"));
	g_signal_connect(G_OBJECT(butt), "clicked", G_CALLBACK(reset2), NULL);
	gtk_table_attach(GTK_TABLE(table), butt, 2, 3, 4, 6, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(butt);
	pr=gtk_button_new_with_label(_("Process\nSpectrum"));
	gtk_table_attach(GTK_TABLE(table), pr, 0, 1, 4, 6, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(pr);
	label=gtk_label_new(_("Inverse Spectrum"));
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), table, label);
	gtk_widget_show(notebook);
	gtk_paned_add1(GTK_PANED(hpane), notebook);
	notebook2=gtk_notebook_new();
	gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook2), GTK_POS_TOP);
	table=gtk_table_new(1, 1, FALSE);
	gtk_widget_show(table);
	plot1=gtk_plot_linear_new();
	{car=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), 1); cag=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), 1); cab=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), 1); caa=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), 1);}
	fll=0;
	{g_array_append_val(car, fll); g_array_append_val(cag, fll); g_array_append_val(cab, fll);}
	fll=1;
	g_array_append_val(caa, fll);
	gtk_plot_set_colour(GTK_PLOT(plot1), car, cag, cab, caa);
	{g_array_unref(car); g_array_unref(cag); g_array_unref(cab); g_array_unref(caa);}
	g_signal_connect(plot1, "moved", G_CALLBACK(pltmv), NULL);
	gtk_widget_show(plot1);
	gtk_table_attach(GTK_TABLE(table), plot1, 0, 1, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK |GTK_EXPAND, 2, 2);
	label=gtk_label_new(_("Spectrum"));
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook2), table, label);
	table=gtk_table_new(1, 1, FALSE);
	gtk_widget_show(table);
	plot2=gtk_plot_linear_new();
	gtk_plot_linear_set_label(GTK_PLOT_LINEAR(plot2), _("Inverse Domain"), _("Magnitude"));
	{car=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), 4); cag=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), 4); cab=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), 4); caa=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), 4);}
	fll=0;
	{g_array_append_val(car, fll); g_array_append_val(cag, fll); g_array_append_val(cag, fll); g_array_append_val(cab, fll); g_array_append_val(cab, fll); g_array_append_val(cab, fll);}
	fll=1;
	{g_array_append_val(car, fll); g_array_append_val(cag, fll); g_array_append_val(cab, fll);}
	fll=0;
	{g_array_append_val(car, fll); g_array_append_val(car, fll); g_array_append_val(cag, fll);}
	fll=0.8;
	{g_array_append_val(caa, fll); g_array_append_val(caa, fll); g_array_append_val(caa, fll); g_array_append_val(caa, fll);}
	gtk_plot_set_colour(GTK_PLOT(plot2), car, cag, cab, caa);
	{g_array_unref(car); g_array_unref(cag); g_array_unref(cab); g_array_unref(caa);}
	g_signal_connect(plot2, "moved", G_CALLBACK(pltmv), NULL);
	gtk_widget_show(plot2);
	gtk_table_attach(GTK_TABLE(table), plot2, 0, 1, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	label=gtk_label_new(_("Inverse Spectrum"));
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook2), table, label);
	rest=gtk_table_new(4, 2, FALSE);
	gtk_widget_show(rest);
	label=gtk_label_new(_("Visibility"));
	gtk_table_attach(GTK_TABLE(rest), label, 0, 1, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(label);
	visl=gtk_label_new("");
	gtk_table_attach(GTK_TABLE(rest), visl, 0, 1, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(visl);
	label=gtk_label_new(_("Domain Shift"));
	gtk_table_attach(GTK_TABLE(rest), label, 1, 2, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(label);
	dsl=gtk_label_new("");
	gtk_table_attach(GTK_TABLE(rest), dsl, 1, 2, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(dsl);
	label=gtk_label_new(_("Chirp"));
	gtk_table_attach(GTK_TABLE(rest), label, 0, 1, 2, 3, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(label);
	chil=gtk_label_new("");
	gtk_table_attach(GTK_TABLE(rest), chil, 0, 1, 3, 4, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(chil);
	label=gtk_label_new(_("Analysis Results"));
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook2), rest, label);
	gtk_widget_show(notebook2);
	gtk_paned_add2(GTK_PANED(hpane), notebook2);
	statusbar=gtk_statusbar_new();
	gtk_box_pack_start(GTK_BOX(vbox), statusbar, FALSE, FALSE, 2);
	gtk_widget_show(statusbar);
	{vis=g_array_new(FALSE, FALSE, sizeof(gdouble)); doms=g_array_new(FALSE, FALSE, sizeof(gdouble)); chp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	{fold=g_strdup("/home"); folr=g_strdup("/home");}
	gtk_widget_show(window);
	gtk_main();
	{g_slist_free(group); g_slist_free(group3); g_slist_free(group4); g_slist_free(group5);}
	if (group2) g_slist_free(group2);
	{g_array_free(bsra, FALSE); g_array_free(bspa, FALSE); g_array_free(isra, FALSE); g_array_free(ispa, FALSE); g_array_free(tca, FALSE); g_array_free(twa, FALSE); g_array_free(zwa, FALSE);}
	{g_array_free(vis, FALSE); g_array_free(doms, FALSE); g_array_free(chp, FALSE);}
	{g_free(fold); g_free(folr);}
	return 0;
}
