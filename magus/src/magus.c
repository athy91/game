/*
 ============================================================================
 Name        : magus.c
 Author      : Battlegod
 Version     :
 Copyright   : Don't copy what's not yours!!!
 Description : GUI mode linker: -Wl -mwindows
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <gtk/gtk.h>
#include <windows.h>
#include <winsock.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#define buffsize 32

WSADATA wsaData;
GtkWidget *window, *logbox, *gamebox, *cbutton, *status, *image;
GdkPixbuf *pixbuf;
GError **error = NULL;
int sock, rec;
struct sockaddr_in server;

static gboolean delete_event(GtkWidget *widget, GdkEvent *event, gpointer data) {
	send(sock, "exit\0", 5, 0);
	closesocket(sock);
	WSACleanup();
	gtk_main_quit();
	return FALSE;
}

void lan(GtkWidget *widget, gpointer data) {

	gchar *buffer = data;

	if (send(sock, buffer, strlen(buffer), 0) != strlen(buffer))
		puts("send() sent a different number of bytes than expected");

	rec = recv(sock, buffer, buffsize - 1, 0);
	buffer[rec] = '\0';

	printf("lan %s\n", buffer);

	gtk_button_set_label((GtkButton *) widget, buffer);

}

void clcon(GtkWidget *widget, gpointer data) {
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (widget))) {
		/* If control reaches here, the toggle button is down */
		if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
			puts("socket() failed");
		/* Establish the connection to the server */
		if (connect(sock, (struct sockaddr *) &server, sizeof(server)) < 0)
			puts("connect() failed");
		gtk_button_set_label((GtkButton *) widget, "connected");
	} else {
		/* If control reaches here, the toggle button is up */
		send(sock, "exit\0", 5, 0);
		closesocket(sock);
		gtk_button_set_label((GtkButton *) widget, "disconnected");
		gtk_widget_hide_all(gamebox);
		gtk_widget_show_all(logbox);
		pixbuf = gdk_pixbuf_new_from_file_at_scale_utf8("pic/0.jpg", 500, 500, TRUE, error);
		gtk_image_set_from_pixbuf( (GtkImage *)image, pixbuf);
	}
	return;
}

void login(GtkWidget *widget, gpointer data) {
	gchar *login = data;
	GtkWidget *popup;

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (status))) {

		if (send(sock, login, strlen(login) + 1, 0) != strlen(login) + 1)
			puts("send() sent a different number of bytes than expected");

		rec = recv(sock, login, buffsize - 1, 0);

		printf("%s\n",login);

		if (!strcmp(login, "TRUE")) {
			return;
		} else {
			gtk_button_clicked((GtkButton *) cbutton);
			gtk_widget_hide_all(logbox);
			gtk_widget_show_all(gamebox);
			return;
		}
	} else {
		popup = gtk_dialog_new_with_buttons("Please connect to the server",
				(GtkWindow *) window, GTK_DIALOG_DESTROY_WITH_PARENT, "OK",
				GTK_STOCK_OK, NULL);
		gtk_widget_show(popup);
	}
}

void set_image(GtkWidget *widget, gpointer data) {
	GdkPixbuf *pixbuf;
	gchar *buffer = data;
	gchar src[10];

	if (send(sock, buffer, strlen(buffer), 0) != strlen(buffer))
		puts("send() sent a different number of bytes than expected");

	rec = recv(sock, src, buffsize - 1, 0);
	src[rec] = '\0';

	printf("image %s\n", src);

	pixbuf = gdk_pixbuf_new_from_file_at_scale_utf8(src, 500, 500, TRUE, error);
	gtk_image_set_from_pixbuf( (GtkImage *)image, pixbuf);

}

int main(int argc, char *argv[]) {
	GtkWidget *logname, *pass, *box, *button, *poz;
	GtkStyle *style;
	GdkPixmap *bg;
	char buffer[buffsize];

	if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0) {
		fprintf(stderr, "WSAStartup() failed");
		exit(1);
	}

	if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		puts("socket() failed");

	memset(&server, 0, sizeof(server)); /* Zero out structure */
	server.sin_family = AF_INET; /* Internet address family */
	server.sin_addr.s_addr = inet_addr("192.168.1.100"); /* Server IP address */
	server.sin_port = htons(21); /* Server port */
	if (connect(sock, (struct sockaddr *) &server, sizeof(server)) < 0) /* Establish the connection to the server */
		puts("connect() failed");

	gtk_init(&argc, &argv);

	pixbuf = gdk_pixbuf_new_from_file_utf8("bgm.jpg", error);
	gdk_pixbuf_render_pixmap_and_mask(pixbuf, &bg, NULL, 0);
	style = gtk_style_new();
	style->bg_pixmap[0] = bg;

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	//gtk_window_set_default_size(GTK_WINDOW(window), 1280, 720);
	pixbuf = gdk_pixbuf_new_from_file_utf8("logo.gif", error);
	gtk_window_set_icon(GTK_WINDOW (window), pixbuf);
	gtk_container_set_border_width(GTK_CONTAINER (window), 10);
	g_signal_connect(window, "delete-event", G_CALLBACK (delete_event), NULL);
	gtk_widget_set_style(GTK_WIDGET(window), GTK_STYLE(style));
	box = gtk_vbox_new(FALSE, 0);
	poz = gtk_hbox_new(FALSE, 0);
	logbox = gtk_vbox_new(FALSE, 0);
	gamebox = gtk_vbox_new(FALSE, 0);
	//image = gtk_image_new_from_file_utf8("maemo.png");
	pixbuf = gdk_pixbuf_new_from_file_at_scale_utf8("pic/0.jpg", 500, 500, TRUE, error);
	image = gtk_image_new_from_pixbuf(pixbuf);
	gtk_container_add(GTK_CONTAINER (poz), image);
	gtk_box_pack_start(GTK_BOX (poz), box, FALSE, FALSE, 10);
	gtk_box_pack_start(GTK_BOX (box), logbox, FALSE, FALSE, 10);
	gtk_box_pack_start(GTK_BOX (box), gamebox, FALSE, FALSE, 10);

	logname = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX (logbox), logname, FALSE, FALSE, 10);
	gtk_widget_show(logname);

	pass = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX (logbox), pass, FALSE, FALSE, 10);
	gtk_widget_show(pass);

	button = gtk_button_new_with_label("login");
	g_signal_connect(button, "clicked", G_CALLBACK (login),
			(gpointer) "athy91");
	gtk_box_pack_start(GTK_BOX (logbox), button, FALSE, FALSE, 10);
	gtk_widget_show(button);

	cbutton = gtk_button_new_with_label(buffer);
	g_signal_connect(cbutton, "clicked", G_CALLBACK (lan), (gpointer) buffer);
	gtk_box_pack_start(GTK_BOX (gamebox), cbutton, FALSE, FALSE, 10);

	button = gtk_button_new_with_label("picture");
	g_signal_connect(button, "clicked", G_CALLBACK (set_image), "pic");
	gtk_box_pack_start(GTK_BOX (gamebox), button, FALSE, FALSE, 10);

	status = gtk_toggle_button_new_with_label("connected");
	gtk_toggle_button_set_active((GtkToggleButton *) status, TRUE);
	g_signal_connect(status, "toggled", G_CALLBACK (clcon), (gpointer) NULL);
	gtk_box_pack_start(GTK_BOX (box), status, FALSE, FALSE, 10);
	gtk_widget_show(status);

	gtk_widget_show(image);
	gtk_widget_show(logbox);
	gtk_widget_show(poz);
	gtk_widget_show(box);
	gtk_container_add(GTK_CONTAINER (window), poz);

	gtk_widget_show(window);
	gtk_main();

	return EXIT_SUCCESS;
}
