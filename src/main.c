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
 * DPR: change to be nonmodal
 * OPD: config writer (+overwrite confirm)
 * OPD: offset tracking.
 * FFT: implement invert to 2pi/x routine
 * PRC: triangle optimisation
 * FFT: window edge correction
 * SAVE: case for batch processed data
 */

#include <gdk/gdkkeysyms.h>
#include <fftw3.h>
#include "main.h"
#include "open.h"
#include "disp.h"
#include "util.h"

GtkWidget *window, *tr, *zpd, *pr, *tracmenu, *trac, *fst, *notebook, *notebook2, *plot1, *plot2, *plot3, *statusbar, *rest, *visl, *dsl, *chil;
GtkWidget *agosa, *agtl, *anosa, *sws, *dlm, *ncmp, *lcmp, *bat, *chi, *twopionx, *opttri, *trans, *dBs, *neg, *wll, *oft;
GtkWidget *bsr, *bsp, *isr, *isp, *tc, *tw, *zw, *jind, *jind2, *kind; /* widgets for windowing */
GArray *bsra, *bspa, *isra, *ispa, *tca, *twa, *zwa, *x, *specs, *yb, *stars, *xsb, *ysb, *sz, *nx, *delf, *vis, *doms, *chp, *msr, *bxr, *byr, *bsz, *bnx; /* arrays for windowing and data */
GSList *group2=NULL; /* list for various traces available */
gint lc, mx; /* number of data points and number of files in batch routine */
guint jdim=0, kdim=0, jdimx=0, kdimx=0, jdimxf=0, kdimxf=0, satl=0, trc=1, flags=0, flagd=0; /* array indices, #of traces, trace number, and current processing state and display flags */
gulong pr_id; /* id for disabling/enabling post-transform processing */

void static prt(GtkWidget *widget, gpointer data)
{
	GtkWidget *wfile;
	GtkFileFilter *filter;
	gchar *str, *fout=NULL;

	switch (gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook2)))
	{
		case 2:
		if ((flags&12)==12)
		{
			wfile=gtk_file_chooser_dialog_new(_("Select Image File"), GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
			g_signal_connect(G_OBJECT(wfile), "destroy", G_CALLBACK(gtk_widget_destroy), G_OBJECT(wfile));
			gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(wfile), TRUE);
			filter=gtk_file_filter_new();
			gtk_file_filter_set_name(filter, "Encapsulated Postscript (EPS)");
			gtk_file_filter_add_pattern(filter, "*.eps");
			gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(wfile), filter);
			if (gtk_dialog_run(GTK_DIALOG(wfile))==GTK_RESPONSE_ACCEPT)
			{
				fout=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(wfile));
				if ((flags&32)!=0) plot_polar_print_eps(plot3, fout);
				else plot_linear_print_eps(plot3, fout);
				g_free(fout);
			}
			gtk_widget_destroy(wfile);
		}
		else if ((flags&2)!=0)
		{
			wfile=gtk_file_chooser_dialog_new(_("Select Image File"), GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
			g_signal_connect(G_OBJECT(wfile), "destroy", G_CALLBACK(gtk_widget_destroy), G_OBJECT(wfile));
			gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(wfile), TRUE);
			filter=gtk_file_filter_new();
			gtk_file_filter_set_name(filter, "Encapsulated Postscript (EPS)");
			gtk_file_filter_add_pattern(filter, "*.eps");
			gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(wfile), filter);
			if (gtk_dialog_run(GTK_DIALOG(wfile))==GTK_RESPONSE_ACCEPT)
			{
				fout=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(wfile));
				plot_linear_print_eps(plot2, fout);
				g_free(fout);
			}
			gtk_widget_destroy(wfile);
		}
		else if ((flags&1)!=0)
		{
			wfile=gtk_file_chooser_dialog_new(_("Select Image File"), GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
			g_signal_connect(G_OBJECT(wfile), "destroy", G_CALLBACK(gtk_widget_destroy), G_OBJECT(wfile));
			gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(wfile), TRUE);
			filter=gtk_file_filter_new();
			gtk_file_filter_set_name(filter, "Encapsulated Postscript (EPS)");
			gtk_file_filter_add_pattern(filter, "*.eps");
			gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(wfile), filter);
			if (gtk_dialog_run(GTK_DIALOG(wfile))==GTK_RESPONSE_ACCEPT)
			{
				fout=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(wfile));
				plot_linear_print_eps(plot1, fout);
				g_free(fout);
			}
			gtk_widget_destroy(wfile);
		}
		else
		{
			gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook2), 0);
			str=g_strdup(_("No available image."));
			gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
			g_free(str);
		}
		break;
		case 1:
		if ((flags&2)!=0)
		{
			wfile=gtk_file_chooser_dialog_new(_("Select Image File"), GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
			g_signal_connect(G_OBJECT(wfile), "destroy", G_CALLBACK(gtk_widget_destroy), G_OBJECT(wfile));
			gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(wfile), TRUE);
			filter=gtk_file_filter_new();
			gtk_file_filter_set_name(filter, "Encapsulated Postscript (EPS)");
			gtk_file_filter_add_pattern(filter, "*.eps");
			gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(wfile), filter);
			if (gtk_dialog_run(GTK_DIALOG(wfile))==GTK_RESPONSE_ACCEPT)
			{
				fout=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(wfile));
				plot_linear_print_eps(plot2, fout);
				g_free(fout);
			}
			gtk_widget_destroy(wfile);
		}
		else if ((flags&1)!=0)
		{
			wfile=gtk_file_chooser_dialog_new(_("Select Image File"), GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
			g_signal_connect(G_OBJECT(wfile), "destroy", G_CALLBACK(gtk_widget_destroy), G_OBJECT(wfile));
			gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(wfile), TRUE);
			filter=gtk_file_filter_new();
			gtk_file_filter_set_name(filter, "Encapsulated Postscript (EPS)");
			gtk_file_filter_add_pattern(filter, "*.eps");
			gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(wfile), filter);
			if (gtk_dialog_run(GTK_DIALOG(wfile))==GTK_RESPONSE_ACCEPT)
			{
				fout=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(wfile));
				plot_linear_print_eps(plot1, fout);
				g_free(fout);
			}
			gtk_widget_destroy(wfile);
		}
		else
		{
			gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook2), 0);
			str=g_strdup(_("No available image."));
			gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
			g_free(str);
		}
		break;
		default:
		if ((flags&1)!=0)
		{
			wfile=gtk_file_chooser_dialog_new(_("Select Image File"), GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
			g_signal_connect(G_OBJECT(wfile), "destroy", G_CALLBACK(gtk_widget_destroy), G_OBJECT(wfile));
			gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(wfile), TRUE);
			filter=gtk_file_filter_new();
			gtk_file_filter_set_name(filter, "Encapsulated Postscript (EPS)");
			gtk_file_filter_add_pattern(filter, "*.eps");
			gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(wfile), filter);
			if (gtk_dialog_run(GTK_DIALOG(wfile))==GTK_RESPONSE_ACCEPT)
			{
				fout=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(wfile));
				plot_linear_print_eps(plot1, fout);
				g_free(fout);
			}
			gtk_widget_destroy(wfile);
		}
		else
		{
			str=g_strdup(_("No available image."));
			gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
			g_free(str);
		}
		break;
	}
}

void static sav(GtkWidget *widget, gpointer data)
{
	GtkWidget *wfile, *dialog, *cont, *label;
	PlotLinear *plt;
	gchar *contents, *str, *str2, *fout=NULL;
	gchar s1[10], s2[10], s3[10];
	gint j, k, sz2;
	gdouble num, num2;
	GError *Err=NULL;

	switch (gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook2)))
	{
		case 2:
		if ((flags&28)==28)
		{
			wfile=gtk_file_chooser_dialog_new(_("Select Data File"), GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
			g_signal_connect(G_OBJECT(wfile), "destroy", G_CALLBACK(gtk_widget_destroy), G_OBJECT(wfile));
			gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(wfile), TRUE);
			if (gtk_dialog_run(GTK_DIALOG(wfile))==GTK_RESPONSE_ACCEPT)
			{
				fout=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(wfile));
				dialog=gtk_dialog_new_with_buttons(_("Parameter selection"), GTK_WINDOW(wfile), GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT, _("Visibility"), 1, _("Domain Shift"), 2, _("Chirp"), 3, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
				cont=gtk_dialog_get_content_area(GTK_DIALOG (dialog));
				label=gtk_label_new(_("Select Parameter to save:"));
				gtk_container_add(GTK_CONTAINER(cont), label);
				switch (gtk_dialog_run(GTK_DIALOG(dialog)))
				{
					case 1:
					/*
					fill contents
					g_file_set_contents(fout, contents, -1, &Err);
					g_free(contents);
					if (Err)
					{
						str=g_strdup_printf(_("Error Saving file: %s."), (gchar *) Err);
						gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
						g_free(str);
						g_error_free(Err);
					}
					 */
					break;
					case 2:
					/*
					fill contents
					g_file_set_contents(fout, contents, -1, &Err);
					g_free(contents);
					if (Err)
					{
						str=g_strdup_printf(_("Error Saving file: %s."), (gchar *) Err);
						gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
						g_free(str);
						g_error_free(Err);
					}
					 */
					break;
					case 3:
					/*
					fill contents
					g_file_set_contents(fout, contents, -1, &Err);
					g_free(contents);
					if (Err)
					{
						str=g_strdup_printf(_("Error Saving file: %s."), (gchar *) Err);
						gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
						g_free(str);
						g_error_free(Err);
					}
					*/
					break;
					default:
					break;
				}
				gtk_widget_destroy(dialog);
				g_free(fout);
			}
			gtk_widget_destroy(wfile);
		}
		else if ((flags&12)==12)
		{
			wfile=gtk_file_chooser_dialog_new(_("Select Data File"), GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
			g_signal_connect(G_OBJECT(wfile), "destroy", G_CALLBACK(gtk_widget_destroy), G_OBJECT(wfile));
			gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(wfile), TRUE);
			if (gtk_dialog_run(GTK_DIALOG(wfile))==GTK_RESPONSE_ACCEPT)
			{
				fout=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(wfile));
				dialog=gtk_dialog_new_with_buttons(_("Parameter selection"), GTK_WINDOW(wfile), GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT, _("Visibility"), 1, _("Domain Shift"), 2, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
				cont=gtk_dialog_get_content_area(GTK_DIALOG (dialog));
				label=gtk_label_new(_("Select Parameter to save:"));
				gtk_container_add(GTK_CONTAINER(cont), label);
				switch (gtk_dialog_run(GTK_DIALOG(dialog)))
				{
					case 1:
					/*
					fill contents
					g_file_set_contents(fout, contents, -1, &Err);
					g_free(contents);
					if (Err)
					{
						str=g_strdup_printf(_("Error Saving file: %s."), (gchar *) Err);
						gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
						g_free(str);
						g_error_free(Err);
					}
					 */
					break;
					case 2:
					/*
					fill contents
					g_file_set_contents(fout, contents, -1, &Err);
					g_free(contents);
					if (Err)
					{
						str=g_strdup_printf(_("Error Saving file: %s."), (gchar *) Err);
						gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
						g_free(str);
						g_error_free(Err);
					}
					 */
					break;
					default:
					break;
				}
				gtk_widget_destroy(dialog);
				g_free(fout);
			}
			gtk_widget_destroy(wfile);
		}
		else if ((flags&2)!=0)
		{
			gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook2), 1);
			wfile=gtk_file_chooser_dialog_new(_("Select Data File"), GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
			g_signal_connect(G_OBJECT(wfile), "destroy", G_CALLBACK(gtk_widget_destroy), G_OBJECT(wfile));
			gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(wfile), TRUE);
			if (gtk_dialog_run(GTK_DIALOG(wfile))==GTK_RESPONSE_ACCEPT)
			{
				fout=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(wfile));
				dialog=gtk_dialog_new_with_buttons(_("Parameter selection"), GTK_WINDOW(wfile), GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT, "Real/Imaginary", 1, "Magnitude/Phase", 2, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
				cont=gtk_dialog_get_content_area(GTK_DIALOG (dialog));
				label=gtk_label_new(_("Select Parameter to save:"));
				gtk_container_add(GTK_CONTAINER(cont), label);
				switch (gtk_dialog_run(GTK_DIALOG(dialog)))
				{
					case 1:
					str2=g_strdup(_("INVERSE_D\tREAL_VAL \tIMAG_VAL "));
					contents=g_strdup(str2);
					for (k=1; k<=jdimxf; k++)
					{
						str=g_strjoin("\t", contents, str2, NULL);
						g_free(contents);
						contents=g_strdup(str);
						g_free(str);
					}
					g_free(str2);
					plt=PLOT_LINEAR(plot2);
					sz2=g_array_index((plt->sizes), gint, 0);
					g_snprintf(s1, 10, "%f", g_array_index(stars, gdouble, 0));
					str2=g_strjoin("\t", "0.0000000", s1, "0.0000000", NULL);
					for (k=1; k<=jdimxf; k++)
					{
						g_snprintf(s1, 10, "%f", g_array_index(stars, gdouble, 2*k*sz2));
						str=g_strjoin("\t", str2, "0.0000000", s1, "0.0000000", NULL);
						g_free(str2);
						str2=g_strdup(str);
						g_free(str);
					}
					str=g_strdup(contents);
					g_free(contents);
					contents=g_strjoin(DLMT, str, str2, NULL);
					g_free(str);
					g_free(str2);
					for (j=1; j<sz2; j++)
					{
						g_snprintf(s1, 10, "%f", j*g_array_index(delf, gdouble, 0));
						g_snprintf(s2, 10, "%f", g_array_index(stars, gdouble, j));
						g_snprintf(s3, 10, "%f", g_array_index(stars, gdouble, (2*sz2)-j));
						str2=g_strjoin("\t", s1, s2, s3, NULL);
						k=1;
						while (k<=jdimxf)
						{
							g_snprintf(s1, 10, "%f", j*g_array_index(delf, gdouble, 0));
							g_snprintf(s2, 10, "%f", g_array_index(stars, gdouble, (2*k*sz2)+j));
							g_snprintf(s3, 10, "%f", g_array_index(stars, gdouble, (2*(++k)*sz2)-j));
							str=g_strjoin("\t", str2, s1, s2, s3, NULL);
							g_free(str2);
							str2=g_strdup(str);
							g_free(str);
						}
						str=g_strdup(contents);
						g_free(contents);
						contents=g_strjoin(DLMT, str, str2, NULL);
						g_free(str);
						g_free(str2);
					}
					g_file_set_contents(fout, contents, -1, &Err);
					g_free(contents);
					if (Err)
					{
						str=g_strdup_printf(_("Error Saving file: %s."), (gchar *) Err);
						gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
						g_free(str);
						g_error_free(Err);
					}
					g_free(fout);
					break;
					case 2:
					str2=g_strdup(_("INVERSE_D\tMAGNITUDE\tPHASE    "));
					contents=g_strdup(str2);
					for (k=1; k<=jdimxf; k++)
					{
						str=g_strjoin("\t", contents, str2, NULL);
						g_free(contents);
						contents=g_strdup(str);
						g_free(str);
					}
					g_free(str2);
					plt=PLOT_LINEAR(plot2);
					sz2=g_array_index((plt->sizes), gint, 0);
					g_snprintf(s1, 10, "%f", g_array_index(stars, gdouble, 0));
					str2=g_strjoin("\t", "0.0000000", s1, "0.0000000", NULL);
					for (k=1; k<=jdimxf; k++)
					{
						g_snprintf(s1, 10, "%f", g_array_index(stars, gdouble, 2*k*sz2));
						str=g_strjoin("\t", str2, "0.0000000", s1, "0.0000000", NULL);
						g_free(str2);
						str2=g_strdup(str);
						g_free(str);
					}
					str=g_strdup(contents);
					g_free(contents);
					contents=g_strjoin(DLMT, str, str2, NULL);
					g_free(str);
					g_free(str2);
					for (j=1; j<sz2; j++)
					{
						g_snprintf(s1, 10, "%f", j*g_array_index(delf, gdouble, 0));
						num=g_array_index(stars, gdouble, j);
						num2=g_array_index(stars, gdouble, (2*sz2)-j);
						g_snprintf(s3, 10, "%f", atan2(num2, num));
						num*=num;
						num2*=num2;
						num+=num2;
						num=sqrt(num);
						g_snprintf(s2, 10, "%f", num);
						str2=g_strjoin("\t", s1, s2, s3, NULL);
						k=1;
						while (k<=jdimxf)
						{
							g_snprintf(s1, 10, "%f", j*g_array_index(delf, gdouble, 0));
							num=g_array_index(stars, gdouble, (2*k*sz2)+j);
							num2=g_array_index(stars, gdouble, (2*(++k)*sz2)-j);
							g_snprintf(s3, 10, "%f", atan2(num2, num));
							num*=num;
							num2*=num2;
							num+=num2;
							num=sqrt(num);
							g_snprintf(s2, 10, "%f", num);
							str=g_strjoin("\t", str2, s1, s2, s3, NULL);
							g_free(str2);
							str2=g_strdup(str);
							g_free(str);
						}
						str=g_strdup(contents);
						g_free(contents);
						contents=g_strjoin(DLMT, str, str2, NULL);
						g_free(str);
						g_free(str2);
					}
					g_file_set_contents(fout, contents, -1, &Err);
					g_free(contents);
					if (Err)
					{
						str=g_strdup_printf(_("Error Saving file: %s."), (gchar *) Err);
						gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
						g_free(str);
						g_error_free(Err);
					}
					g_free(fout);
					break;
					default:
					break;
				}
			}
			gtk_widget_destroy(wfile);
		}
		else
		{
			gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook2), 0);
			str=g_strdup(_("No available processed data."));
			gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
			g_free(str);
		}
		break;
		case 1:
		if ((flags&2)!=0)
		{
			wfile=gtk_file_chooser_dialog_new(_("Select Data File"), GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
			g_signal_connect(G_OBJECT(wfile), "destroy", G_CALLBACK(gtk_widget_destroy), G_OBJECT(wfile));
			gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(wfile), TRUE);
			if (gtk_dialog_run(GTK_DIALOG(wfile))==GTK_RESPONSE_ACCEPT)
			{
				fout=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(wfile));
				dialog=gtk_dialog_new_with_buttons(_("Parameter selection"), GTK_WINDOW(wfile), GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT, "Real/Imaginary", 1, "Magnitude/Phase", 2, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
				cont=gtk_dialog_get_content_area(GTK_DIALOG (dialog));
				label=gtk_label_new(_("Select Parameter to save:"));
				gtk_container_add(GTK_CONTAINER(cont), label);
				switch (gtk_dialog_run(GTK_DIALOG(dialog)))
				{
					case 1:
					str2=g_strdup(_("INVERSE_D\tREAL_VAL \tIMAG_VAL "));
					contents=g_strdup(str2);
					for (k=1; k<=jdimxf; k++)
					{
						str=g_strjoin("\t", contents, str2, NULL);
						g_free(contents);
						contents=g_strdup(str);
						g_free(str);
					}
					g_free(str2);
					plt=PLOT_LINEAR(plot2);
					sz2=g_array_index((plt->sizes), gint, 0);
					g_snprintf(s1, 10, "%f", g_array_index(stars, gdouble, 0));
					str2=g_strjoin("\t", "0.0000000", s1, "0.0000000", NULL);
					for (k=1; k<=jdimxf; k++)
					{
						g_snprintf(s1, 10, "%f", g_array_index(stars, gdouble, 2*k*sz2));
						str=g_strjoin("\t", str2, "0.0000000", s1, "0.0000000", NULL);
						g_free(str2);
						str2=g_strdup(str);
						g_free(str);
					}
					str=g_strdup(contents);
					g_free(contents);
					contents=g_strjoin(DLMT, str, str2, NULL);
					g_free(str);
					g_free(str2);
					for (j=1; j<sz2; j++)
					{
						g_snprintf(s1, 10, "%f", j*g_array_index(delf, gdouble, 0));
						g_snprintf(s2, 10, "%f", g_array_index(stars, gdouble, j));
						g_snprintf(s3, 10, "%f", g_array_index(stars, gdouble, (2*sz2)-j));
						str2=g_strjoin("\t", s1, s2, s3, NULL);
						k=1;
						while (k<=jdimxf)
						{
							g_snprintf(s1, 10, "%f", j*g_array_index(delf, gdouble, 0));
							g_snprintf(s2, 10, "%f", g_array_index(stars, gdouble, (2*k*sz2)+j));
							g_snprintf(s3, 10, "%f", g_array_index(stars, gdouble, (2*(++k)*sz2)-j));
							str=g_strjoin("\t", str2, s1, s2, s3, NULL);
							g_free(str2);
							str2=g_strdup(str);
							g_free(str);
						}
						str=g_strdup(contents);
						g_free(contents);
						contents=g_strjoin(DLMT, str, str2, NULL);
						g_free(str);
						g_free(str2);
					}
					g_file_set_contents(fout, contents, -1, &Err);
					g_free(contents);
					if (Err)
					{
						str=g_strdup_printf(_("Error Saving file: %s."), (gchar *) Err);
						gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
						g_free(str);
						g_error_free(Err);
					}
					g_free(fout);
					break;
					case 2:
					str2=g_strdup(_("INVERSE_D\tMAGNITUDE\tPHASE    "));
					contents=g_strdup(str2);
					for (k=1; k<=jdimxf; k++)
					{
						str=g_strjoin("\t", contents, str2, NULL);
						g_free(contents);
						contents=g_strdup(str);
						g_free(str);
					}
					g_free(str2);
					plt=PLOT_LINEAR(plot2);
					sz2=g_array_index((plt->sizes), gint, 0);
					g_snprintf(s1, 10, "%f", g_array_index(stars, gdouble, 0));
					str2=g_strjoin("\t", "0.0000000", s1, "0.0000000", NULL);
					for (k=1; k<=jdimxf; k++)
					{
						g_snprintf(s1, 10, "%f", g_array_index(stars, gdouble, 2*k*sz2));
						str=g_strjoin("\t", str2, "0.0000000", s1, "0.0000000", NULL);
						g_free(str2);
						str2=g_strdup(str);
						g_free(str);
					}
					str=g_strdup(contents);
					g_free(contents);
					contents=g_strjoin(DLMT, str, str2, NULL);
					g_free(str);
					g_free(str2);
					for (j=1; j<sz2; j++)
					{
						g_snprintf(s1, 10, "%f", j*g_array_index(delf, gdouble, 0));
						num=g_array_index(stars, gdouble, j);
						num2=g_array_index(stars, gdouble, (2*sz2)-j);
						g_snprintf(s3, 10, "%f", atan2(num2, num));
						num*=num;
						num2*=num2;
						num+=num2;
						num=sqrt(num);
						g_snprintf(s2, 10, "%f", num);
						str2=g_strjoin("\t", s1, s2, s3, NULL);
						k=1;
						while (k<=jdimxf)
						{
							g_snprintf(s1, 10, "%f", j*g_array_index(delf, gdouble, 0));
							num=g_array_index(stars, gdouble, (2*k*sz2)+j);
							num2=g_array_index(stars, gdouble, (2*(++k)*sz2)-j);
							g_snprintf(s3, 10, "%f", atan2(num2, num));
							num*=num;
							num2*=num2;
							num+=num2;
							num=sqrt(num);
							g_snprintf(s2, 10, "%f", num);
							str=g_strjoin("\t", str2, s1, s2, s3, NULL);
							g_free(str2);
							str2=g_strdup(str);
							g_free(str);
						}
						str=g_strdup(contents);
						g_free(contents);
						contents=g_strjoin(DLMT, str, str2, NULL);
						g_free(str);
						g_free(str2);
					}
					g_file_set_contents(fout, contents, -1, &Err);
					g_free(contents);
					if (Err)
					{
						str=g_strdup_printf(_("Error Saving file: %s."), (gchar *) Err);
						gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
						g_free(str);
						g_error_free(Err);
					}
					g_free(fout);
					break;
					default:
					break;
				}
			}
			gtk_widget_destroy(wfile);
		}
		else
		{
			gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook2), 0);
			str=g_strdup(_("No available processed data."));
			gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
			g_free(str);
		}
		break;
		default:
		if ((flags&2)!=0)
		{
			gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook2), 1);
			wfile=gtk_file_chooser_dialog_new(_("Select Data File"), GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
			g_signal_connect(G_OBJECT(wfile), "destroy", G_CALLBACK(gtk_widget_destroy), G_OBJECT(wfile));
			gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(wfile), TRUE);
			if (gtk_dialog_run(GTK_DIALOG(wfile))==GTK_RESPONSE_ACCEPT)
			{
				fout=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(wfile));
				dialog=gtk_dialog_new_with_buttons(_("Parameter selection"), GTK_WINDOW(wfile), GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT, "Real/Imaginary", 1, "Magnitude/Phase", 2, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
				cont=gtk_dialog_get_content_area(GTK_DIALOG (dialog));
				label=gtk_label_new(_("Select Parameter to save:"));
				gtk_container_add(GTK_CONTAINER(cont), label);
				switch (gtk_dialog_run(GTK_DIALOG(dialog)))
				{
					case 1:
					str2=g_strdup(_("INVERSE_D\tREAL_VAL \tIMAG_VAL "));
					contents=g_strdup(str2);
					for (k=1; k<=jdimxf; k++)
					{
						str=g_strjoin("\t", contents, str2, NULL);
						g_free(contents);
						contents=g_strdup(str);
						g_free(str);
					}
					g_free(str2);
					plt=PLOT_LINEAR(plot2);
					sz2=g_array_index((plt->sizes), gint, 0);
					g_snprintf(s1, 10, "%f", g_array_index(stars, gdouble, 0));
					str2=g_strjoin("\t", "0.0000000", s1, "0.0000000", NULL);
					for (k=1; k<=jdimxf; k++)
					{
						g_snprintf(s1, 10, "%f", g_array_index(stars, gdouble, 2*k*sz2));
						str=g_strjoin("\t", str2, "0.0000000", s1, "0.0000000", NULL);
						g_free(str2);
						str2=g_strdup(str);
						g_free(str);
					}
					str=g_strdup(contents);
					g_free(contents);
					contents=g_strjoin(DLMT, str, str2, NULL);
					g_free(str);
					g_free(str2);
					for (j=1; j<sz2; j++)
					{
						g_snprintf(s1, 10, "%f", j*g_array_index(delf, gdouble, 0));
						g_snprintf(s2, 10, "%f", g_array_index(stars, gdouble, j));
						g_snprintf(s3, 10, "%f", g_array_index(stars, gdouble, (2*sz2)-j));
						str2=g_strjoin("\t", s1, s2, s3, NULL);
						k=1;
						while (k<=jdimxf)
						{
							g_snprintf(s1, 10, "%f", j*g_array_index(delf, gdouble, 0));
							g_snprintf(s2, 10, "%f", g_array_index(stars, gdouble, (2*k*sz2)+j));
							g_snprintf(s3, 10, "%f", g_array_index(stars, gdouble, (2*(++k)*sz2)-j));
							str=g_strjoin("\t", str2, s1, s2, s3, NULL);
							g_free(str2);
							str2=g_strdup(str);
							g_free(str);
						}
						str=g_strdup(contents);
						g_free(contents);
						contents=g_strjoin(DLMT, str, str2, NULL);
						g_free(str);
						g_free(str2);
					}
					g_file_set_contents(fout, contents, -1, &Err);
					g_free(contents);
					if (Err)
					{
						str=g_strdup_printf(_("Error Saving file: %s."), (gchar *) Err);
						gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
						g_free(str);
						g_error_free(Err);
					}
					g_free(fout);
					break;
					case 2:
					str2=g_strdup(_("INVERSE_D\tMAGNITUDE\tPHASE    "));
					contents=g_strdup(str2);
					for (k=1; k<=jdimxf; k++)
					{
						str=g_strjoin("\t", contents, str2, NULL);
						g_free(contents);
						contents=g_strdup(str);
						g_free(str);
					}
					g_free(str2);
					plt=PLOT_LINEAR(plot2);
					sz2=g_array_index((plt->sizes), gint, 0);
					g_snprintf(s1, 10, "%f", g_array_index(stars, gdouble, 0));
					str2=g_strjoin("\t", "0.0000000", s1, "0.0000000", NULL);
					for (k=1; k<=jdimxf; k++)
					{
						g_snprintf(s1, 10, "%f", g_array_index(stars, gdouble, 2*k*sz2));
						str=g_strjoin("\t", str2, "0.0000000", s1, "0.0000000", NULL);
						g_free(str2);
						str2=g_strdup(str);
						g_free(str);
					}
					str=g_strdup(contents);
					g_free(contents);
					contents=g_strjoin(DLMT, str, str2, NULL);
					g_free(str);
					g_free(str2);
					for (j=1; j<sz2; j++)
					{
						g_snprintf(s1, 10, "%f", j*g_array_index(delf, gdouble, 0));
						num=g_array_index(stars, gdouble, j);
						num2=g_array_index(stars, gdouble, (2*sz2)-j);
						g_snprintf(s3, 10, "%f", atan2(num2, num));
						num*=num;
						num2*=num2;
						num+=num2;
						num=sqrt(num);
						g_snprintf(s2, 10, "%f", num);
						str2=g_strjoin("\t", s1, s2, s3, NULL);
						k=1;
						while (k<=jdimxf)
						{
							g_snprintf(s1, 10, "%f", j*g_array_index(delf, gdouble, 0));
							num=g_array_index(stars, gdouble, (2*k*sz2)+j);
							num2=g_array_index(stars, gdouble, (2*(++k)*sz2)-j);
							g_snprintf(s3, 10, "%f", atan2(num2, num));
							num*=num;
							num2*=num2;
							num+=num2;
							num=sqrt(num);
							g_snprintf(s2, 10, "%f", num);
							str=g_strjoin("\t", str2, s1, s2, s3, NULL);
							g_free(str2);
							str2=g_strdup(str);
							g_free(str);
						}
						str=g_strdup(contents);
						g_free(contents);
						contents=g_strjoin(DLMT, str, str2, NULL);
						g_free(str);
						g_free(str2);
					}
					g_file_set_contents(fout, contents, -1, &Err);
					g_free(contents);
					if (Err)
					{
						str=g_strdup_printf(_("Error Saving file: %s."), (gchar *) Err);
						gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
						g_free(str);
						g_error_free(Err);
					}
					g_free(fout);
					break;
					default:
					break;
				}
			}
			gtk_widget_destroy(wfile);
		}
		else
		{
			str=g_strdup(_("No available processed data."));
			gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
			g_free(str);
		}
		break;
	}
}

void static prs(GtkWidget *widget, gpointer data)
{
	GtkWidget *label;
	PlotLinear *plt;
	gint j, k, l, st, sp, sz2;
	gdouble idelf, iv, vzt, vt, ivd, ivdt, tcn, twd, phi, phio, phia, dst, ddp, pn, cn, tp, ct;
	gchar *str;
	gchar s[10];

	if ((flags&2)!=0)
	{
		plt=PLOT_LINEAR(plot2);
		sz2=g_array_index((plt->sizes), gint, 0);/* check placing of this with what is desired for multiplots (within for loop?) */
		g_array_free(vis, TRUE);
		g_array_free(doms, TRUE);
		vis=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimxf*kdimx);
		doms=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimxf*kdimx);
		if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(opttri)))
		{
			if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(chi)))
			{
				g_array_free(chp, TRUE);
				chp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimxf*kdimx);
				for (j=0; j<=jdimxf; j++)
				{
					iv=g_array_index(delf, gdouble, j);
					if (iv<DZE) idelf=G_MAXDOUBLE;
					else idelf=1/iv;
					/*
					 fit values to zwa
					 */
					vzt=g_array_index(stars, gdouble, 2*j*sz2);
					iv=g_array_index(zwa, gdouble, j)*idelf/2;
					for (l=1; l<iv; l++)
					{
						ivd=g_array_index(stars, gdouble, l+(2*j*sz2));
						ivd*=ivd;
						ivdt=g_array_index(stars, gdouble, (2*(j+1)*sz2)-l);
						ivdt*=ivdt;
						vzt+=sqrt(ivd+ivdt);
					}
					if (vzt<DZE) vzt=G_MAXDOUBLE;
					else vzt=l/vzt;
					for (k=0; k<=kdimx; k++)
					{
						st=ceil(g_array_index(isra, gdouble, j+(k*jdimxf))*idelf);
						sp=floor(g_array_index(ispa, gdouble, j+(k*jdimxf))*idelf);
						/*
						 fit values to twa and tca
						 */
						tcn=g_array_index(tca, gdouble, j+(k*jdimxf))*idelf;
						twd=g_array_index(twa, gdouble, j+(k*jdimxf))*idelf/2;
						if ((st<(sz2-2))&&(sp<sz2)&&((sp-st)>1))
						{
							vt=g_array_index(stars, gdouble, st+(2*j*sz2));
							ivdt=g_array_index(stars, gdouble, (2*(j+1)*sz2)-st);
							phia=atan2(ivdt, vt);
							vt*=vt;
							ivdt*=ivdt;
							vt=sqrt(vt+ivdt);
							ivd=g_array_index(stars, gdouble, st+1+(2*j*sz2));
							ivdt=g_array_index(stars, gdouble, (2*(j+1)*sz2)-st-1);
							phio=-atan2(ivdt, ivd);
							phia+=phio;
							ivd*=ivd;
							ivdt*=ivdt;
							vt+=sqrt(ivd+ivdt);
							{pn=0; cn=0; dst=0; ddp=0;}
							for (l=st+2; l<=sp; l++)
							{
								ivd=g_array_index(stars, gdouble, l+(2*j*sz2));
								ivdt=g_array_index(stars, gdouble, (2*(j+1)*sz2)-l);
								phi=atan2(ivdt, ivd);
								phio+=phi;
								if (phio>G_PI) phio-=(MY_2PI);
								else if (phio<=NMY_PI) phio+=(MY_2PI);
								if (l>(tcn-twd+0.5))
								{
									if ((l-1)<=(tcn-twd))
									{
										tp=(((gdouble) l)-tcn-0.5)/twd;
										pn+=++tp;
										dst+=tp*phio;
									}
									else if (l<=(tcn+0.5))
									{
										tp=(((gdouble) l)-tcn-0.5)/twd;
										ct=(((gdouble) l)-tcn-1)/twd;
										pn+=++tp;
										dst+=tp*phio;
										cn+=++ct;
										phia+=phio;
										ddp+=ct*phia;
									}
									else if ((l-1)<=tcn)
									{
										tp=(tcn+0.5-((gdouble) l))/twd;
										ct=(((gdouble) l)-tcn-1)/twd;
										pn+=++tp;
										dst+=tp*phio;
										cn+=++ct;
										phia+=phio;
										ddp+=ct*phia;
									}
									else if (l<(tcn+twd+0.5))
									{
										tp=(tcn+0.5-((gdouble) l))/twd;
										ct=(tcn+1-((gdouble) l))/twd;
										pn+=++tp;
										dst+=tp*phio;
										cn+=++ct;
										phia+=phio;
										ddp+=ct*phia;
									}
									else if ((l-1)<(tcn+twd))
									{
										ct=(tcn+1-((gdouble) l))/twd;
										cn+=++ct;
										phia+=phio;
										ddp+=ct*phia;
									}
								}
								phia=-phio;
								phio=-phi;
								ivd*=ivd;
								ivdt*=ivdt;
								vt+=sqrt(ivd+ivdt);
							}
							pn*=NMY_2PI*g_array_index(delf, gdouble, j);
							if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
							else dst/=pn;
							cn*=NMY_2PI*g_array_index(delf, gdouble, j);
							cn*=G_PI*g_array_index(delf, gdouble, j);
							if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
							else ddp=cn/ddp;
							vt*=vzt/(sp-st+1);
						}
						else
						{
							str=g_strdup_printf(_("Insufficient windowing range in channel %d, %d."), j, k);
							gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
							g_free(str);
							{vt=0; dst=0; ddp=0;}
						}
						g_array_append_val(vis, vt);
						g_array_append_val(doms, dst);
						g_array_append_val(chp, ddp);
					}
				}
				label=gtk_label_new(_("Chirp"));
				gtk_table_attach(GTK_TABLE(rest), label, 0, 1, 2, 3, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
				gtk_widget_show(label);
				if (jdim<=jdimxf)
				{
					vt=g_array_index(vis, gdouble, (jdim+(kdim*jdimxf)));
					g_snprintf(s, 7, "%f", vt);
					gtk_label_set_text(GTK_LABEL(visl), s);
					vt=g_array_index(doms, gdouble, (jdim+(kdim*jdimxf)));
					g_snprintf(s, 9, "%f", vt);
					gtk_label_set_text(GTK_LABEL(dsl), s);
					vt=g_array_index(chp, gdouble, (jdim+(kdim*jdimxf)));
					g_snprintf(s, 8, "%f", vt);
					chil=gtk_label_new(s);					
				}
				else
				{
					str=g_strdup("");
					gtk_label_set_text(GTK_LABEL(visl), str);
					gtk_label_set_text(GTK_LABEL(dsl), str);
					chil=gtk_label_new(str);
					g_free(str);
				}
				gtk_table_attach(GTK_TABLE(rest), chil, 0, 1, 3, 4, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
				gtk_widget_show(chil);
				flags|=20;
			}
			else
			{
				for (j=0; j<=jdimxf; j++)
				{
					iv=g_array_index(delf, gdouble, j);
					if (iv<DZE) idelf=G_MAXDOUBLE;
					else idelf=1/iv;
					/*
					 fit values to zwa
					 */
					vzt=g_array_index(stars, gdouble, 2*j*sz2);
					iv=g_array_index(zwa, gdouble, j)*idelf/2;
					for (l=1; l<iv; l++)
					{
						ivd=g_array_index(stars, gdouble, l+(2*j*sz2));
						ivd*=ivd;
						ivdt=g_array_index(stars, gdouble, (2*(j+1)*sz2)-l);
						ivdt*=ivdt;
						vzt+=sqrt(ivd+ivdt);
					}
					if (vzt<DZE) vzt=G_MAXDOUBLE;
					else vzt=l/vzt;
					for (k=0; k<=kdimx; k++)
					{
						st=ceil(g_array_index(isra, gdouble, j+(k*jdimxf))*idelf);
						sp=floor(g_array_index(ispa, gdouble, j+(k*jdimxf))*idelf);
						/*
						 fit values to twa and tca
						 */
						tcn=g_array_index(tca, gdouble, j+(k*jdimxf))*idelf;
						twd=g_array_index(twa, gdouble, j+(k*jdimxf))*idelf/2;
						if ((st<(sz2-1))&&(sp<sz2)&&((sp-st)>0))
						{
							vt=g_array_index(stars, gdouble, st+(2*j*sz2));
							ivdt=g_array_index(stars, gdouble, (2*(j+1)*sz2)-st);
							phio=-atan2(ivdt, vt);
							vt*=vt;
							ivdt*=ivdt;
							vt=sqrt(vt+ivdt);
							{dst=0; pn=0;}
							for (l=st+1; l<=sp; l++)
							{
								ivd=g_array_index(stars, gdouble, l+(2*j*sz2));
								ivdt=g_array_index(stars, gdouble, (2*(j+1)*sz2)-l);
								phi=atan2(ivdt, ivd);
								phio+=phi;
								if (phio>G_PI) phio-=(MY_2PI);
								else if (phio<=NMY_PI) phio+=(MY_2PI);
								if (l>(tcn-twd+0.5))
								{
									if (l<=(tcn+0.5))
									{
										tp=(((gdouble) l)-tcn-0.5)/twd;
										pn+=++tp;
										dst+=tp*phio;
									}
									else if (l<(tcn+twd+0.5))
									{
										tp=(tcn+0.5-((gdouble) l))/twd;
										pn+=++tp;
										dst+=tp*phio;
									}
								}
								phio=-phi;
								ivd*=ivd;
								ivdt*=ivdt;
								vt+=sqrt(ivd+ivdt);
							}
							pn*=MY_2PI*g_array_index(delf, gdouble, j);
							dst=-dst;
							dst=dst/pn;
							vt*=vzt/(sp-st+1);
						}
						else
						{
							str=g_strdup_printf(_("Insufficient windowing range in channel %d, %d."), j, k);
							gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
							g_free(str);
							{vt=0; dst=0;}
						}
						g_array_append_val(vis, vt);
						g_array_append_val(doms, dst);
					}
				}
				if (jdim<=jdimxf)
				{
					vt=g_array_index(vis, gdouble, (jdim+(kdim*jdimxf)));
					g_snprintf(s, 7, "%f", vt);
					gtk_label_set_text(GTK_LABEL(visl), s);
					vt=g_array_index(doms, gdouble, (jdim+(kdim*jdimxf)));
					g_snprintf(s, 9, "%f", vt);
					gtk_label_set_text(GTK_LABEL(dsl), s);				
				}
				else
				{
					str=g_strdup("");
					gtk_label_set_text(GTK_LABEL(visl), str);
					gtk_label_set_text(GTK_LABEL(dsl), str);
					g_free(str);
				}
				flags|=4;
			}
		}
		else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(chi)))
		{
			g_array_free(chp, TRUE);
			chp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), (jdimxf*kdimx));
			for (j=0; j<=jdimxf; j++)
			{
				vzt=g_array_index(stars, gdouble, 2*j*sz2);
				iv=g_array_index(delf, gdouble, j);
				if (iv<DZE) idelf=G_MAXDOUBLE;
				else idelf=1/iv;
				iv=g_array_index(zwa, gdouble, j)*idelf/2;
				for (l=1; l<iv; l++)
				{
					ivd=g_array_index(stars, gdouble, l+(2*j*sz2));
					ivd*=ivd;
					ivdt=g_array_index(stars, gdouble, (2*(j+1)*sz2)-l);
					ivdt*=ivdt;
					vzt+=sqrt(ivd+ivdt);
				}
				if (vzt<DZE) vzt=G_MAXDOUBLE;
				else vzt=l/vzt;
				for (k=0; k<=kdimx; k++)
				{
					st=ceil(g_array_index(isra, gdouble, j+(k*jdimxf))*idelf);
					sp=floor(g_array_index(ispa, gdouble, j+(k*jdimxf))*idelf);
					tcn=g_array_index(tca, gdouble, j+(k*jdimxf))*idelf;
					twd=g_array_index(twa, gdouble, j+(k*jdimxf))*idelf/2;
					if ((st<(sz2-2))&&(sp<sz2)&&((sp-st)>1))
					{
						vt=g_array_index(stars, gdouble, st+(2*j*sz2));
						ivdt=g_array_index(stars, gdouble, (2*(j+1)*sz2)-st);
						phia=atan2(ivdt, vt);
						vt*=vt;
						ivdt*=ivdt;
						vt=sqrt(vt+ivdt);
						ivd=g_array_index(stars, gdouble, st+1+(2*j*sz2));
						ivdt=g_array_index(stars, gdouble, (2*(j+1)*sz2)-st-1);
						phio=-atan2(ivdt, ivd);
						phia+=phio;
						ivd*=ivd;
						ivdt*=ivdt;
						vt+=sqrt(ivd+ivdt);
						{pn=0; cn=0; dst=0; ddp=0;}
						for (l=st+2; l<=sp; l++)
						{
							ivd=g_array_index(stars, gdouble, l+(2*j*sz2));
							ivdt=g_array_index(stars, gdouble, (2*(j+1)*sz2)-l);
							phi=atan2(ivdt, ivd);
							phio+=phi;
							if (phio>G_PI) phio-=(MY_2PI);
							else if (phio<=NMY_PI) phio+=(MY_2PI);
							if (l>(tcn-twd+0.5))
							{
								if ((l-1)<=(tcn-twd))
								{
									tp=(((gdouble) l)-tcn-0.5)/twd;
									pn+=++tp;
									dst+=tp*phio;
								}
								else if (l<=(tcn+0.5))
								{
									tp=(((gdouble) l)-tcn-0.5)/twd;
									ct=(((gdouble) l)-tcn-1)/twd;
									pn+=++tp;
									dst+=tp*phio;
									cn+=++ct;
									phia+=phio;
									ddp+=ct*phia;
								}
								else if ((l-1)<=tcn)
								{
									tp=(tcn+0.5-((gdouble) l))/twd;
									ct=(((gdouble) l)-tcn-1)/twd;
									pn+=++tp;
									dst+=tp*phio;
									cn+=++ct;
									phia+=phio;
									ddp+=ct*phia;
								}
								else if (l<(tcn+twd+0.5))
								{
									tp=(tcn+0.5-((gdouble) l))/twd;
									ct=(tcn+1-((gdouble) l))/twd;
									pn+=++tp;
									dst+=tp*phio;
									cn+=++ct;
									phia+=phio;
									ddp+=ct*phia;
								}
								else if ((l-1)<(tcn+twd))
								{
									ct=(tcn+1-((gdouble) l))/twd;
									cn+=++ct;
									phia+=phio;
									ddp+=ct*phia;
								}
							}
							phia=-phio;
							phio=-phi;
							ivd*=ivd;
							ivdt*=ivdt;
							vt+=sqrt(ivd+ivdt);
						}
						pn*=NMY_2PI*g_array_index(delf, gdouble, j);
						if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
						else dst/=pn;
						cn*=NMY_2PI*g_array_index(delf, gdouble, j);
						cn*=G_PI*g_array_index(delf, gdouble, j);
						if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
						else ddp=cn/ddp;
						vt*=vzt/(sp-st+1);
					}
					else
					{
						str=g_strdup_printf(_("Insufficient windowing range in channel %d, %d."), j, k);
						gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
						g_free(str);
						{vt=0; dst=0; ddp=0;}
					}
					g_array_append_val(vis, vt);
					g_array_append_val(doms, dst);
					g_array_append_val(chp, ddp);
				}
			}
			label=gtk_label_new(_("Chirp"));
			gtk_table_attach(GTK_TABLE(rest), label, 0, 1, 2, 3, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
			gtk_widget_show(label);
			if (jdim<=jdimxf)
			{
				vt=g_array_index(vis, gdouble, (jdim+(kdim*jdimxf)));
				g_snprintf(s, 7, "%f", vt);
				gtk_label_set_text(GTK_LABEL(visl), s);
				vt=g_array_index(doms, gdouble, (jdim+(kdim*jdimxf)));
				g_snprintf(s, 9, "%f", vt);
				gtk_label_set_text(GTK_LABEL(dsl), s);
				vt=g_array_index(chp, gdouble, (jdim+(kdim*jdimxf)));
				g_snprintf(s, 8, "%f", vt);
				chil=gtk_label_new(s);					
			}
			else
			{
				str=g_strdup("");
				gtk_label_set_text(GTK_LABEL(visl), str);
				gtk_label_set_text(GTK_LABEL(dsl), str);
				chil=gtk_label_new(str);
				g_free(str);
			}
			flags|=20;
		}
		else
		{
			for (j=0; j<=jdimxf; j++)
			{
				vzt=g_array_index(stars, gdouble, 2*j*sz2);
				iv=g_array_index(delf, gdouble, j);
				if (iv<DZE) idelf=G_MAXDOUBLE;
				else idelf=1/iv;
				iv=g_array_index(zwa, gdouble, j)*idelf/2;
				for (l=1; l<iv; l++)
				{
					ivd=g_array_index(stars, gdouble, l+(2*j*sz2));
					ivd*=ivd;
					ivdt=g_array_index(stars, gdouble, (2*(j+1)*sz2)-l);
					ivdt*=ivdt;
					vzt+=sqrt(ivd+ivdt);
				}
				if (vzt<DZE) vzt=G_MAXDOUBLE;
				else vzt=l/vzt;
				for (k=0; k<=kdimx; k++)
				{
					st=ceil(g_array_index(isra, gdouble, j+(k*jdimxf))*idelf);
					sp=floor(g_array_index(ispa, gdouble, j+(k*jdimxf))*idelf);
					tcn=g_array_index(tca, gdouble, j+(k*jdimxf))*idelf;
					twd=g_array_index(twa, gdouble, j+(k*jdimxf))*idelf/2;
					if ((st<(sz2-1))&&(sp<sz2)&&((sp-st)>0))
					{
						vt=g_array_index(stars, gdouble, st+(2*j*sz2));
						ivdt=g_array_index(stars, gdouble, (2*(j+1)*sz2)-st);
						phio=-atan2(ivdt, vt);
						vt*=vt;
						ivdt*=ivdt;
						vt=sqrt(vt+ivdt);
						{dst=0; pn=0;}
						for (l=st+1; l<=sp; l++)
						{
							ivd=g_array_index(stars, gdouble, l+(2*j*sz2));
							ivdt=g_array_index(stars, gdouble, (2*(j+1)*sz2)-l);
							phi=atan2(ivdt, ivd);
							phio+=phi;
							if (phio>G_PI) phio-=(MY_2PI);
							else if (phio<=NMY_PI) phio+=(MY_2PI);
							if (l>(tcn-twd+0.5))
							{
								if (l<=(tcn+0.5))
								{
									tp=(((gdouble) l)-tcn-0.5)/twd;
									pn+=++tp;
									dst+=tp*phio;
								}
								else if (l<(tcn+twd+0.5))
								{
									tp=(tcn+0.5-((gdouble) l))/twd;
									pn+=++tp;
									dst+=tp*phio;
								}
							}
							phio=-phi;
							ivd*=ivd;
							ivdt*=ivdt;
							vt+=sqrt(ivd+ivdt);
						}
						pn*=NMY_2PI*g_array_index(delf, gdouble, j);
						if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
						else dst/=pn;
						vt*=vzt/(sp-st+1);
					}
					else
					{
						str=g_strdup_printf(_("Insufficient windowing range in channel %d, %d."), j, k);
						gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
						g_free(str);
						{vt=0; dst=0;}
					}
					g_array_append_val(vis, vt);
					g_array_append_val(doms, dst);
				}
			}
			if (jdim<=jdimxf)
			{
				vt=g_array_index(vis, gdouble, (jdim+(kdim*jdimxf)));
				g_snprintf(s, 7, "%f", vt);
				gtk_label_set_text(GTK_LABEL(visl), s);
				vt=g_array_index(doms, gdouble, (jdim+(kdim*jdimxf)));
				g_snprintf(s, 9, "%f", vt);
				gtk_label_set_text(GTK_LABEL(dsl), s);				
			}
			else
			{
				str=g_strdup("");
				gtk_label_set_text(GTK_LABEL(visl), str);
				gtk_label_set_text(GTK_LABEL(dsl), str);
				g_free(str);
			}
			flags|=4;
		}
		kdimxf=kdimx;
		gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook2), 2);
	}
	else
	{
		str=g_strdup(_("No transform for analysis exists yet."));
		gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
		g_free(str);
	}
}

void trs(GtkWidget *widget, gpointer data) /* need to incorporate case for inversion to 2pi/x */
{
	GtkAdjustment *adj;
	PlotLinear *plt;
	guint j, k, st, sp;
	gint n, zp, dx, dx2;
	gdouble iv, clc, ofs, xx, yx;
	gchar *str;
	double *y, *star;
	fftw_plan p;
	fftw_r2r_kind type=FFTW_R2HC;

	if ((flags&1)!=0)
	{
		ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
		zp=1<<(gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(zpd)));
		n=zp*(jdimx+1);
		g_array_free(stars, TRUE);
		g_array_free(xsb, TRUE);
		g_array_free(ysb, TRUE);
		g_array_free(delf, TRUE);
		delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), (jdimx+1));
		y=fftw_malloc(sizeof(double)*n);
		for (j=0; j<n; j++) y[j]=0;
		if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(twopionx)))
		{
			if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(lcmp)))
			{
				if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(dBs)))
				{
					if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans)))
					{
						if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdBss- */
						{
							for (j=0; j<=jdimx; j++)
							{
								iv=g_array_index(bsra, gdouble, j);
								k=0;
								while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
								st=k;
								iv=g_array_index(bspa, gdouble, j);
								while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
								sp=k-st;
								if (sp>zp)
								{
									str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
									gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
									g_free(str);
									sp=zp;
								}
								/* fill array */
							}
						}
						else /* +TdBss- */
						{
							for (j=0; j<=jdimx; j++)
							{
								iv=g_array_index(bsra, gdouble, j);
								k=0;
								while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
								st=k;
								iv=g_array_index(bspa, gdouble, j);
								while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
								sp=k-st;
								if (sp>zp)
								{
									str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
									gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
									g_free(str);
									sp=zp;
								}
								/* fill array */
							}
						}
					}
					else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdBss- */
					{
						for (j=0; j<=jdimx; j++)
						{
							iv=g_array_index(bsra, gdouble, j);
							k=0;
							while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
							st=k;
							iv=g_array_index(bspa, gdouble, j);
							while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
							sp=k-st;
							if (sp>zp)
							{
								str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
								gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
								g_free(str);
								sp=zp;
							}
							/* fill array */
						}
					}
					else /* +RdBss- */
					{
						for (j=0; j<=jdimx; j++)
						{
							iv=g_array_index(bsra, gdouble, j);
							k=0;
							while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
							st=k;
							iv=g_array_index(bspa, gdouble, j);
							while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
							sp=k-st;
							if (sp>zp)
							{
								str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
								gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
								g_free(str);
								sp=zp;
							}
							/* fill array */
						}
					}
				}
				else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tlss- and +Tlss- */
				{
					for (j=0; j<=jdimx; j++)
					{
						iv=g_array_index(bsra, gdouble, j);
						k=0;
						while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
						st=k;
						iv=g_array_index(bspa, gdouble, j);
						while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
						sp=k-st;
						if (sp>zp)
						{
							str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
							gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
							g_free(str);
							sp=zp;
						}
						/* fill array */
					}
				}
				else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -Rlss- */
				{
					for (j=0; j<=jdimx; j++)
					{
						iv=g_array_index(bsra, gdouble, j);
						k=0;
						while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
						st=k;
						iv=g_array_index(bspa, gdouble, j);
						while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
						sp=k-st;
						if (sp>zp)
						{
							str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
							gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
							g_free(str);
							sp=zp;
						}
						/* fill array */
					}
				}
				else /* +Rlss- */
				{
					for (j=0; j<=jdimx; j++)
					{
						iv=g_array_index(bsra, gdouble, j);
						k=0;
						while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
						st=k;
						iv=g_array_index(bspa, gdouble, j);
						while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
						sp=k-st;
						if (sp>zp)
						{
							str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
							gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
							g_free(str);
							sp=zp;
						}
						/* fill array */
					}
				}
			}
			else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(dBs)))
			{
				if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans)))
				{
					if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdB0- */
					{
						for (j=0; j<=jdimx; j++)
						{
							iv=g_array_index(bsra, gdouble, j);
							k=0;
							while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
							st=k;
							iv=g_array_index(bspa, gdouble, j);
							while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
							sp=k-st;
							if (sp>zp)
							{
								str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
								gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
								g_free(str);
								sp=zp;
							}
							/* fill array */
						}
					}
					else /* +TdB0- */
					{
						for (j=0; j<=jdimx; j++)
						{
							iv=g_array_index(bsra, gdouble, j);
							k=0;
							while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
							st=k;
							iv=g_array_index(bspa, gdouble, j);
							while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
							sp=k-st;
							if (sp>zp)
							{
								str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
								gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
								g_free(str);
								sp=zp;
							}
							/* fill array */
						}
					}
				}
				else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdB0- */
				{
					for (j=0; j<=jdimx; j++)
					{
						iv=g_array_index(bsra, gdouble, j);
						k=0;
						while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
						st=k;
						iv=g_array_index(bspa, gdouble, j);
						while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
						sp=k-st;
						if (sp>zp)
						{
							str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
							gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
							g_free(str);
							sp=zp;
						}
						/* fill array */
					}
				}
				else /* +RdB0- */
				{
					for (j=0; j<=jdimx; j++)
					{
						iv=g_array_index(bsra, gdouble, j);
						k=0;
						while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
						st=k;
						iv=g_array_index(bspa, gdouble, j);
						while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
						sp=k-st;
						if (sp>zp)
						{
							str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
							gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
							g_free(str);
							sp=zp;
						}
						/* fill array */
					}
				}
			}
			else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tl0- and +Tl0- */
			{
				for (j=0; j<=jdimx; j++)
				{
					iv=g_array_index(bsra, gdouble, j);
					k=0;
					while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
					st=k;
					iv=g_array_index(bspa, gdouble, j);
					while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
					sp=k-st;
					if (sp>zp)
					{
						str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
						gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
						g_free(str);
						sp=zp;
					}
					/* fill array */
				}
			}
			else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -Rl0- */
			{
				for (j=0; j<=jdimx; j++)
				{
					iv=g_array_index(bsra, gdouble, j);
					k=0;
					while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
					st=k;
					iv=g_array_index(bspa, gdouble, j);
					while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
					sp=k-st;
					if (sp>zp)
					{
						str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
						gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
						g_free(str);
						sp=zp;
					}
					/* fill array */
				}
			}
			else /* +Rl0- */
			{
				for (j=0; j<=jdimx; j++)
				{
					iv=g_array_index(bsra, gdouble, j);
					k=0;
					while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
					st=k;
					iv=g_array_index(bspa, gdouble, j);
					while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
					sp=k-st;
					if (sp>zp)
					{
						str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
						gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
						g_free(str);
						sp=zp;
					}
					/* fill array */
				}
			}
		}
		else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(lcmp)))
		{
			if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(dBs)))
			{
				if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans)))
				{
					if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdBss+ */
					{
						for (j=0; j<=jdimx; j++)
						{
							iv=g_array_index(bsra, gdouble, j);
							k=0;
							while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
							st=k;
							iv=g_array_index(bspa, gdouble, j);
							while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
							sp=k-st;
							if (sp>zp)
							{
								str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
								gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
								g_free(str);
								sp=zp;
							}
							iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
							g_array_append_val(delf, iv);
							for (k=0; k<sp; k++)
							{
								clc=ofs-g_array_index(specs, gdouble, trc-1+((k+st)*satl));
								if (clc<0)
								{
									clc=-exp(LNTOT*clc);
									y[k+(j*zp)]=log(++clc);
								}
								else y[k+(j*zp)]=-G_MAXDOUBLE;
							}
						}
					}
					else /* +TdBss+ */
					{
						for (j=0; j<=jdimx; j++)
						{
							iv=g_array_index(bsra, gdouble, j);
							k=0;
							while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
							st=k;
							iv=g_array_index(bspa, gdouble, j);
							while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
							sp=k-st;
							if (sp>zp)
							{
								str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
								gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
								g_free(str);
								sp=zp;
							}
							iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
							g_array_append_val(delf, iv);
							for (k=0; k<sp; k++)
							{
								clc=g_array_index(specs, gdouble, trc-1+((k+st)*satl))-ofs;
								if (clc<0)
								{
									clc=-exp(LNTOT*clc);
									y[k+(j*zp)]=log(++clc);
								}
								else y[k+(j*zp)]=-G_MAXDOUBLE;
							}
						}
					}
				}
				else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdBss+ */
				{
					for (j=0; j<=jdimx; j++)
					{
						iv=g_array_index(bsra, gdouble, j);
						k=0;
						while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
						st=k;
						iv=g_array_index(bspa, gdouble, j);
						while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
						sp=k-st;
						if (sp>zp)
						{
							str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
							gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
							g_free(str);
							sp=zp;
						}
						iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
						g_array_append_val(delf, iv);
						for (k=0; k<sp; k++) y[k+(j*zp)]=0.1*(ofs-g_array_index(specs, gdouble, trc-1+((k+st)*satl)));
					}
				}
				else /* +RdBss+ */
				{
					for (j=0; j<=jdimx; j++)
					{
						iv=g_array_index(bsra, gdouble, j);
						k=0;
						while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
						st=k;
						iv=g_array_index(bspa, gdouble, j);
						while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
						sp=k-st;
						if (sp>zp)
						{
							str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
							gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
							g_free(str);
							sp=zp;
						}
						iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
						g_array_append_val(delf, iv);
						for (k=0; k<sp; k++) y[k+(j*zp)]=0.1*(g_array_index(specs, gdouble, trc-1+((k+st)*satl))-ofs);
					}
				}
			}
			else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tlss+ +Tlss+ */
			{
				if ((ofs<DZE)&&(ofs>NZE))
				{
					str=g_strdup(_("Offset must be nonzero for linear measurements."));
					gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
					g_free(str);
				}
				else
				{
					for (j=0; j<=jdimx; j++)
					{
						iv=g_array_index(bsra, gdouble, j);
						k=0;
						while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
						st=k;
						iv=g_array_index(bspa, gdouble, j);
						while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
						sp=k-st;
						if (sp>zp)
						{
							str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
							gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
							g_free(str);
							sp=zp;
						}
						iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
						g_array_append_val(delf, iv);
						for (k=0; k<sp; k++)
						{
							clc=-g_array_index(specs, gdouble, trc-1+((k+st)*satl))/ofs;
							clc++;
							if (clc>0) y[k+(j*zp)]=log(clc);
							else y[k+(j*zp)]=-G_MAXDOUBLE;
						}
					}
				}
			}
			else /* -Rlss+ +Rlss+ */
			{
				if ((ofs<DZE)&&(ofs>NZE))
				{
					str=g_strdup(_("Offset must be nonzero for linear measurements."));
					gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
					g_free(str);
				}
				else
				{
					for (j=0; j<=jdimx; j++)
					{
						iv=g_array_index(bsra, gdouble, j);
						k=0;
						while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
						st=k;
						iv=g_array_index(bspa, gdouble, j);
						while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
						sp=k-st;
						if (sp>zp)
						{
							str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
							gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
							g_free(str);
							sp=zp;
						}
						iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
						g_array_append_val(delf, iv);
						for (k=0; k<sp; k++)
						{
							clc=g_array_index(specs, gdouble, trc-1+((k+st)*satl))/ofs;
							if (clc>0) y[k+(j*zp)]=log(clc);
							else y[k+(j*zp)]=-G_MAXDOUBLE;
						}
					}
				}
			}
		}
		else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(dBs)))
		{
			if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans)))
			{
				if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdB0+ */
				{
					for (j=0; j<=jdimx; j++)
					{
						iv=g_array_index(bsra, gdouble, j);
						k=0;
						while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
						st=k;
						iv=g_array_index(bspa, gdouble, j);
						while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
						sp=k-st;
						if (sp>zp)
						{
							str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
							gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
							g_free(str);
							sp=zp;
						}
						iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
						g_array_append_val(delf, iv);
						for (k=0; k<sp; k++)
						{
							clc=ofs-g_array_index(specs, gdouble, trc-1+((k+st)*satl));
							clc=-exp(LNTOT*clc);
							y[k+(j*zp)]=++clc;
						}
					}
				}
				else /* +TdB0+ */
				{
					j=0;
					for (j=0; j<=jdimx; j++)
					{
						iv=g_array_index(bsra, gdouble, j);
						k=0;
						while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
						st=k;
						iv=g_array_index(bspa, gdouble, j);
						while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
						sp=k-st;
						if (sp>zp)
						{
							str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
							gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
							g_free(str);
							sp=zp;
						}
						iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
						g_array_append_val(delf, iv);
						for (k=0; k<sp; k++)
						{
							clc=g_array_index(specs, gdouble, trc-1+((k+st)*satl))-ofs;
							clc=-exp(LNTOT*clc);
							y[k+(j*zp)]=++clc;
						}
					}
				}
			}
			else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdB0+ */
			{
				for (j=0; j<=jdimx; j++)
				{
					iv=g_array_index(bsra, gdouble, j);
					k=0;
					while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
					st=k;
					iv=g_array_index(bspa, gdouble, j);
					while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
					sp=k-st;
					if (sp>zp)
					{
						str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
						gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
						g_free(str);
						sp=zp;
					}
					iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
					g_array_append_val(delf, iv);
					for (k=0; k<sp; k++) y[k+(j*zp)]=exp(LNTOT*(ofs-g_array_index(specs, gdouble, trc-1+((k+st)*satl))));
				}
			}
			else /* +RdB0+ */
			{
				for (j=0; j<=jdimx; j++)
				{
					iv=g_array_index(bsra, gdouble, j);
					k=0;
					while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
					st=k;
					iv=g_array_index(bspa, gdouble, j);
					while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
					sp=k-st;
					if (sp>zp)
					{
						str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
						gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
						g_free(str);
						sp=zp;
					}
					iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
					g_array_append_val(delf, iv);
					for (k=0; k<sp; k++) y[k+(j*zp)]=exp(LNTOT*(g_array_index(specs, gdouble, trc-1+((k+st)*satl))-ofs));
				}
			}
		}
		else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tl0+ and +Tl0+ */ 
		{
			if ((ofs<DZE)&&(ofs>NZE))
			{
				str=g_strdup(_("Offset must be nonzero for linear measurements."));
				gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
				g_free(str);
			}
			else
			{
				for (j=0; j<=jdimx; j++)
				{
					iv=g_array_index(bsra, gdouble, j);
					k=0;
					while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
					st=k;
					iv=g_array_index(bspa, gdouble, j);
					while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
					sp=k-st;
					if (sp>zp)
					{
						str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
						gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
						g_free(str);
						sp=zp;
					}
					iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
					g_array_append_val(delf, iv);
					for (k=0; k<sp; k++)
					{
						clc=-g_array_index(specs, gdouble, trc-1+((k+st)*satl))/ofs;
						y[k+(j*zp)]=++clc;
					}
				}
			}
		}
		else /* -Rl0+ and +Rl0+ */ 
		{
			if ((ofs<DZE)&&(ofs>NZE))
			{
				str=g_strdup(_("Offset must be nonzero for linear measurements."));
				gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
				g_free(str);
			}
			else
			{
				for (j=0; j<=jdimx; j++)
				{
					iv=g_array_index(bsra, gdouble, j);
					k=0;
					while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
					st=k;
					iv=g_array_index(bspa, gdouble, j);
					while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
					sp=k-st;
					if (sp>zp)
					{
						str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
						gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
						g_free(str);
						sp=zp;
					}
					iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
					g_array_append_val(delf, iv);
					for (k=0; k<sp; k++) y[k+(j*zp)]=g_array_index(specs, gdouble, trc-1+((k+st)*satl))/ofs;
				}
			}
		}
		star=fftw_malloc(sizeof(double)*n);
		p=fftw_plan_many_r2r(1, &zp, (jdimx+1), y, NULL, 1, zp, star, NULL, 1, zp, &type, FFTW_ESTIMATE);
		fftw_execute(p);
		fftw_destroy_plan(p);
		fftw_free(y);
		stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
		for (j=0; j<n; j++)
		{
			iv=star[j];
			g_array_append_val(stars, iv);
		}
		xsb=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), zp/2);
		sz=g_array_new(FALSE, FALSE, sizeof(gint));
		nx=g_array_new(FALSE, FALSE, sizeof(gint));
		dx=zp/2;
		dx2=0;
		if ((flagd&1)==0)
		{
			for (j=0; j<zp/2; j++)
			{
				xx=j*g_array_index(delf, gdouble, 0);
				g_array_append_val(xsb, xx);
			}
			ysb=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), zp/2);
			yx=fabs(star[0]);
			g_array_append_val(ysb, yx);
			for (j=1; j<(zp/2); j++)
			{
				iv=star[j];
				iv*=iv;
				clc=star[zp-j];
				clc*=clc;
				iv+=clc;
				iv=sqrt(iv);
				if (yx<iv) yx=iv;
				g_array_append_val(ysb, iv);
			}
			g_array_append_val(sz, dx);
			g_array_append_val(nx, dx2);
		}
		else
		{
			ysb=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n/2);
			xx=0;
			g_array_append_val(xsb, xx);
			yx=fabs(star[0]);
			g_array_append_val(ysb, yx);
			for (j=1; j<zp/2; j++)
			{
				xx=j*g_array_index(delf, gdouble, 0);
				g_array_append_val(xsb, xx);
				iv=star[j];
				iv*=iv;
				clc=star[zp-j];
				clc*=clc;
				iv+=clc;
				iv=sqrt(iv);
				if (yx<iv) yx=iv;
				g_array_append_val(ysb, iv);
			}
			g_array_append_val(sz, dx);
			g_array_append_val(nx, dx2);
			for (k=1; k<=jdimx; k++)
			{
				xx=0;
				g_array_append_val(xsb, xx);
				iv=fabs(star[k*zp]);
				g_array_append_val(ysb, iv);
				if (yx<iv) yx=iv;
				for (j=1; j<zp/2; j++)
				{
					xx=j*g_array_index(delf, gdouble, 0);
					g_array_append_val(xsb, xx);
					iv=star[j+(k*zp)];
					iv*=iv;
					clc=star[((k+1)*zp)-j];
					clc*=clc;
					iv+=clc;
					iv=sqrt(iv);
					if (yx<iv) yx=iv;
					g_array_append_val(ysb, iv);
				}
				g_array_append_val(sz, dx);
				dx2+=dx;
				g_array_append_val(nx, dx2);
			}
		}
		fftw_free(star);
		plt=PLOT_LINEAR(plot2);
		(plt->sizes)=sz;
		(plt->ind)=nx;
		(plt->xdata)=xsb;
		(plt->ydata)=ysb;
		plot_linear_update_scale_pretty(plot2, 0, xx, 0, yx);
		gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), 1);
		gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook2), 1);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(jind), 0);
		adj=(GtkAdjustment *) gtk_adjustment_new(0, 0, jdimx, 1.0, 5.0, 0.0);
		gtk_spin_button_set_adjustment(GTK_SPIN_BUTTON(jind2), adj);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(jind2), 0);
		adj=(GtkAdjustment *) gtk_adjustment_new(0, 0, G_MAXINT8, 1.0, 5.0, 0.0);
		gtk_spin_button_set_adjustment(GTK_SPIN_BUTTON(kind), adj);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(kind), 0);
		jdimxf=jdimx;
		flags|=2;
		pr_id=g_signal_connect(G_OBJECT(pr), "clicked", G_CALLBACK(prs), NULL);
	}
	else
	{
		str=g_strdup(_("Open a file for analysis first."));
		gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
		g_free(str);
	}
}

int main( int argc, char *argv[])
{
	GtkAdjustment *adj;
	GtkWidget *vbox, *mnb, *mnu, *smnu, *mni, *hpane, *table, *label, *butt;
	GtkAccelGroup *accel_group=NULL;
	GSList *group=NULL, *group3=NULL;
	gdouble fll=0;

	bindtextdomain(PACKAGE, LOCALEDIR);
	bind_textdomain_codeset(PACKAGE, "UTF-8");
	textdomain(PACKAGE);
	if (!g_thread_supported()) g_thread_init(NULL);
	gdk_threads_init();
	gdk_threads_enter();
	gtk_init(&argc, &argv);
	window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), _("Harmonic Spectrum Analyser"));
	g_signal_connect_swapped(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);
	vbox=gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window), vbox);
	gtk_widget_show(vbox);
	mnb=gtk_menu_bar_new();
	gtk_box_pack_start(GTK_BOX(vbox), mnb, FALSE, FALSE, 2);
	gtk_widget_show(mnb);
	mnu=gtk_menu_new();
	accel_group=gtk_accel_group_new();
	gtk_window_add_accel_group(GTK_WINDOW(window), accel_group);
	mni=gtk_image_menu_item_new_from_stock(GTK_STOCK_OPEN, NULL);
	gtk_widget_add_accelerator(mni, "activate", accel_group, GDK_o, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	g_signal_connect(G_OBJECT(mni), "activate", G_CALLBACK(opd), NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(mnu), mni);
	gtk_widget_show(mni);
	mni=gtk_image_menu_item_new_from_stock(GTK_STOCK_SAVE, NULL);
	gtk_widget_add_accelerator(mni, "activate", accel_group, GDK_s, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	g_signal_connect(G_OBJECT(mni), "activate", G_CALLBACK(sav), NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(mnu), mni);
	gtk_widget_show(mni);
	mni=gtk_image_menu_item_new_from_stock(GTK_STOCK_PRINT, NULL);
	gtk_widget_add_accelerator(mni, "activate", accel_group, GDK_p, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	g_signal_connect(G_OBJECT(mni), "activate", G_CALLBACK(prt), NULL);
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
	bat=gtk_check_menu_item_new_with_label(_("Batch Process Data?"));
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(bat), FALSE);
	gtk_menu_shell_append(GTK_MENU_SHELL(mnu), bat);
	gtk_widget_show(bat);
	oft=gtk_check_menu_item_new_with_label(_("Autotrack Offset?"));
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(oft), FALSE);
	gtk_menu_shell_append(GTK_MENU_SHELL(mnu), oft);
	gtk_widget_show(oft);
	mni=gtk_menu_item_new_with_mnemonic(_("_Advanced"));
	gtk_widget_show(mni);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(mni), mnu);
	gtk_menu_shell_append(GTK_MENU_SHELL(mnb), mni);
	mnu=gtk_menu_new();
	mni=gtk_menu_item_new_with_label(_("Instructions"));
	gtk_widget_add_accelerator(mni, "activate", accel_group, GDK_F1, 0, GTK_ACCEL_VISIBLE);
	g_signal_connect(G_OBJECT(mni), "activate", G_CALLBACK(help), NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(mnu), mni);
	gtk_widget_show(mni);
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
	bsra=g_array_new(FALSE, FALSE, sizeof(gdouble));
	bspa=g_array_new(FALSE, FALSE, sizeof(gdouble));
	isra=g_array_new(FALSE, FALSE, sizeof(gdouble));
	ispa=g_array_new(FALSE, FALSE, sizeof(gdouble));
	tca=g_array_new(FALSE, FALSE, sizeof(gdouble));
	twa=g_array_new(FALSE, FALSE, sizeof(gdouble));
	zwa=g_array_new(FALSE, FALSE, sizeof(gdouble));
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
	adj=(GtkAdjustment *) gtk_adjustment_new(0, -G_MAXDOUBLE, G_MAXDOUBLE, 1.0, 5.0, 0.0);
	bsr=gtk_spin_button_new(adj, 0.5, 3);
	g_signal_connect(G_OBJECT(bsr), "value-changed", G_CALLBACK(upa1), (gpointer) bsra);
	gtk_table_attach(GTK_TABLE(table), bsr, 0, 1, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(bsr);
	label=gtk_label_new(_("Spectrum Stop:"));
	gtk_table_attach(GTK_TABLE(table), label, 1, 2, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(label);
	adj=(GtkAdjustment *) gtk_adjustment_new(1, -G_MAXDOUBLE, G_MAXDOUBLE, 1.0, 5.0, 0.0);
	bsp=gtk_spin_button_new(adj, 0.5, 3);
	g_signal_connect(G_OBJECT(bsp), "value-changed", G_CALLBACK(upa2), (gpointer) bspa);
	gtk_table_attach(GTK_TABLE(table), bsp, 1, 2, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(bsp);
	label=gtk_label_new(_("Offset:"));
	gtk_table_attach(GTK_TABLE(table), label, 1, 2, 2, 3, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(label);
	adj=(GtkAdjustment *) gtk_adjustment_new(1, -G_MAXDOUBLE, G_MAXDOUBLE, 1.0, 5.0, 0.0);
	fst=gtk_spin_button_new(adj, 0.5, 2);
	gtk_table_attach(GTK_TABLE(table), fst, 1, 2, 3, 4, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(fst);
	label=gtk_label_new(_("j index:"));
	gtk_table_attach(GTK_TABLE(table), label, 2, 3, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(label);
	adj=(GtkAdjustment *) gtk_adjustment_new(0, 0, G_MAXINT8, 1.0, 5.0, 0.0);
	jind=gtk_spin_button_new(adj, 0, 0);
	g_signal_connect(G_OBJECT(jind), "value-changed", G_CALLBACK(upj), NULL);
	gtk_table_attach(GTK_TABLE(table), jind, 2, 3, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(jind);
	label=gtk_label_new(_("Zero Padding 2^:"));
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, 2, 3, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(label);
	adj=(GtkAdjustment *) gtk_adjustment_new(12, 0, 31, 1.0, 5.0, 0.0);
	zpd=gtk_spin_button_new(adj, 0, 0);
	gtk_table_attach(GTK_TABLE(table), zpd, 0, 1, 3, 4, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(zpd);
	butt=gtk_button_new_with_label(_("Reset\nArrays"));
	g_signal_connect(G_OBJECT(butt), "clicked", G_CALLBACK(reset), NULL);
	gtk_table_attach(GTK_TABLE(table), butt, 2, 3, 2, 4, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(butt);
	tr=gtk_button_new_with_label(_("Transform Spectrum"));
	gtk_table_attach(GTK_TABLE(table), tr, 0, 3, 4, 5, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(tr);
	label=gtk_label_new(_("Spectrum"));
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), table, label);
	table=gtk_table_new(6, 3, FALSE);
	gtk_widget_show(table);
	label=gtk_label_new(_("Inverse Spectrum Start:"));
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(label);
	adj=(GtkAdjustment *) gtk_adjustment_new(1, -G_MAXDOUBLE, G_MAXDOUBLE, 1.0, 5.0, 0.0);
	isr=gtk_spin_button_new(adj, 0.5, 3);
	g_signal_connect(G_OBJECT(isr), "value-changed", G_CALLBACK(upa2), (gpointer) isra);
	gtk_table_attach(GTK_TABLE(table), isr, 0, 1, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(isr);
	label=gtk_label_new(_("Inverse Spectrum Stop:"));
	gtk_table_attach(GTK_TABLE(table), label, 1, 2, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(label);
	adj=(GtkAdjustment *) gtk_adjustment_new(3, -G_MAXDOUBLE, G_MAXDOUBLE, 1.0, 5.0, 0.0);
	isp=gtk_spin_button_new(adj, 0.5, 3);
	g_signal_connect(G_OBJECT(isp), "value-changed", G_CALLBACK(upa2), (gpointer) ispa);
	gtk_table_attach(GTK_TABLE(table), isp, 1, 2, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(isp);
	label=gtk_label_new(_("Triangle Centre:"));
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, 2, 3, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(label);
	adj=(GtkAdjustment *) gtk_adjustment_new(2, -G_MAXDOUBLE, G_MAXDOUBLE, 1.0, 5.0, 0.0);
	tc=gtk_spin_button_new(adj, 0.5, 3);
	g_signal_connect(G_OBJECT(tc), "value-changed", G_CALLBACK(upa2), (gpointer) tca);
	gtk_table_attach(GTK_TABLE(table), tc, 0, 1, 3, 4, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(tc);
	label=gtk_label_new(_("Triangle Full Width:"));
	gtk_table_attach(GTK_TABLE(table), label, 1, 2, 2, 3, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(label);
	adj=(GtkAdjustment *) gtk_adjustment_new(2, DZE, G_MAXDOUBLE, 1.0, 5.0, 0.0);
	tw=gtk_spin_button_new(adj, 0.5, 3);
	g_signal_connect(G_OBJECT(tw), "value-changed", G_CALLBACK(upa2), (gpointer) twa);
	gtk_table_attach(GTK_TABLE(table), tw, 1, 2, 3, 4, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(tw);
	label=gtk_label_new(_("DC Peak Width:"));
	gtk_table_attach(GTK_TABLE(table), label, 1, 2, 4, 5, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(label);
	adj=(GtkAdjustment *) gtk_adjustment_new(2, DZE, G_MAXDOUBLE, 1.0, 5.0, 0.0);
	zw=gtk_spin_button_new(adj, 0.5, 3);
	g_signal_connect(G_OBJECT(zw), "value-changed", G_CALLBACK(upa1), (gpointer) zwa);
	gtk_table_attach(GTK_TABLE(table), zw, 1, 2, 5, 6, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(zw);
	label=gtk_label_new(_("j index:"));
	gtk_table_attach(GTK_TABLE(table), label, 2, 3, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(label);
	adj=(GtkAdjustment *) gtk_adjustment_new(0, 0, 0, 1.0, 5.0, 0.0);
	jind2=gtk_spin_button_new(adj, 0, 0);
	g_signal_connect(G_OBJECT(jind2), "value-changed", G_CALLBACK(upj), NULL);
	gtk_table_attach(GTK_TABLE(table), jind2, 2, 3, 1, 2, GTK_FILL|GTK_SHRINK, GTK_FILL|GTK_SHRINK, 2, 2);
	gtk_widget_show(jind2);
	label=gtk_label_new(_("k index:"));
	gtk_table_attach(GTK_TABLE(table), label, 2, 3, 2, 3, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(label);
	adj=(GtkAdjustment *) gtk_adjustment_new(0, 0, 0, 1.0, 5.0, 0.0);
	kind=gtk_spin_button_new(adj, 0, 0);
	g_signal_connect(G_OBJECT(kind), "value-changed", G_CALLBACK(upk), NULL);
	gtk_table_attach(GTK_TABLE(table), kind, 2, 3, 3, 4, GTK_FILL|GTK_SHRINK, GTK_FILL|GTK_SHRINK, 2, 2);
	gtk_widget_show(kind);
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
	plot1=plot_linear_new();
	g_signal_connect(plot1, "moved", G_CALLBACK(pltmv), NULL);
	gtk_widget_show(plot1);
	gtk_table_attach(GTK_TABLE(table), plot1, 0, 1, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK |GTK_EXPAND, 2, 2);
	label=gtk_label_new(_("Spectrum"));
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook2), table, label);
	table=gtk_table_new(1, 1, FALSE);
	gtk_widget_show(table);
	plot2=plot_linear_new();
	((PLOT_LINEAR(plot2))->xlab)=g_strdup(_("Inverse Domain"));
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
	label=gtk_label_new(_("Analysis Results"));
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook2), rest, label);
	gtk_widget_show(notebook2);
	gtk_paned_add2(GTK_PANED(hpane), notebook2);
	statusbar=gtk_statusbar_new();
	gtk_box_pack_start(GTK_BOX(vbox), statusbar, FALSE, FALSE, 2);
	gtk_widget_show(statusbar);
	x=g_array_new(FALSE, FALSE, sizeof(gdouble));
	yb=g_array_new(FALSE, FALSE, sizeof(gdouble));
	specs=g_array_new(FALSE, FALSE, sizeof(gdouble));
	stars=g_array_new(FALSE, FALSE, sizeof(gdouble));
	xsb=g_array_new(FALSE, FALSE, sizeof(gdouble));
	ysb=g_array_new(FALSE, FALSE, sizeof(gdouble));
	delf=g_array_new(FALSE, FALSE, sizeof(gdouble));
	vis=g_array_new(FALSE, FALSE, sizeof(gdouble));
	doms=g_array_new(FALSE, FALSE, sizeof(gdouble));
	chp=g_array_new(FALSE, FALSE, sizeof(gdouble));
	msr=g_array_new(FALSE, FALSE, sizeof(gdouble));
	gtk_widget_show(window);
	gtk_main();
	gdk_threads_leave();
	return 0;
}
