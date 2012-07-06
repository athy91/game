/*
 ============================================================================
 Name        : magus.c
 Author      : Athy91
 Version     :
 Copyright   :
 Description : maybe later
 Note	 	 : GUI mode linker: -Wl -mwindows
 ============================================================================
 */
/*
 * INCLUDES
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h> 	//for strings
#include <gtk/gtk.h>	//GTK+
#include <windows.h>
#include <winsock.h>	//for network
#include <gdk-pixbuf/gdk-pixbuf.h>//for pictures
/*
 * DEFINITIONS
 */
#define buffsize 32
#define LN 1			// Number of locations -> game mechanics
/*
 * TYPE DEFINITIONS
 */
typedef struct birth_date {     //birth date -> need to be separated for age
	int year;
	int month;
	int day;
} BORN;

typedef struct character {     //character data -> game mechanics
	BORN born;
	char name[31], House[31], Gender;
	int height, weight, age, rep, str, dex, inte;
	int spd, vit, wis, sta, cha, hly;
	int kl[LN], pt[20], tech[20], abil[20], talent[20];
} caracter;
/*
 * DECLARATIONS
 */
WSADATA wsaData;
GtkWidget *pass, *logname, *window, *logbox, *gamebox, *cbutton, *status,
		*image, *stats, *name, *house, *gender, *born, *height, *weight;
GdkPixbuf *pixbuf;
GError **error = NULL;
caracter user;
int sock, rec;
struct sockaddr_in server;
/*
 * FUNCTIONS
 */
static gboolean delete_event(GtkWidget *widget, GdkEvent *event, gpointer data) {     //close window when X is pressed
	send(sock, "exit\0", 5, 0);
	closesocket(sock);
	WSACleanup();
	gtk_main_quit();
	return FALSE;
}

GtkWidget *make_stats() {
	GtkWidget *stat, *hbox, *label;

	stat = gtk_vbox_new(FALSE, 0);
	hbox = gtk_hbox_new(FALSE, 0);

	label = gtk_label_new("Name:");
	gtk_box_pack_start(GTK_BOX (hbox), label, TRUE, FALSE, 10);
	house = gtk_label_new(user.House);
	gtk_box_pack_start(GTK_BOX (hbox), house, FALSE, FALSE, 10);
	name = gtk_label_new(user.name);
	gtk_box_pack_start(GTK_BOX (hbox), name, FALSE, FALSE, 10);
	gtk_box_pack_start(GTK_BOX (stat), hbox, FALSE, FALSE, 10);

	hbox = gtk_hbox_new(FALSE, 0);
	label = gtk_label_new("Gender:");
	gtk_box_pack_start(GTK_BOX (hbox), label, TRUE, FALSE, 10);
	gender = gtk_label_new(NULL);
	gtk_box_pack_start(GTK_BOX (hbox), gender, FALSE, FALSE, 10);
	label = gtk_label_new("Birth date:");
	gtk_box_pack_start(GTK_BOX (hbox), label, TRUE, FALSE, 10);
	born = gtk_label_new(NULL);
	gtk_box_pack_start(GTK_BOX (hbox), born, FALSE, FALSE, 10);
	gtk_box_pack_start(GTK_BOX (stat), hbox, FALSE, FALSE, 10);

	hbox = gtk_hbox_new(FALSE, 0);
	label = gtk_label_new("Height:");
	gtk_box_pack_start(GTK_BOX (hbox), label, TRUE, FALSE, 10);
	height = gtk_label_new(NULL);
	gtk_box_pack_start(GTK_BOX (hbox), height, FALSE, FALSE, 10);
	label = gtk_label_new("Weight:");
	gtk_box_pack_start(GTK_BOX (hbox), label, TRUE, FALSE, 10);
	weight = gtk_label_new(NULL);
	gtk_box_pack_start(GTK_BOX (hbox), weight, FALSE, FALSE, 10);
	gtk_box_pack_start(GTK_BOX (stat), hbox, FALSE, FALSE, 10);

	return stat;
}

void err(gchar *message) {     //error popup
	GtkWidget *dialog, *label, *content_area;
	dialog = gtk_dialog_new_with_buttons("Error",
			(GtkWindow *) window,     //create popup
			GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL, GTK_STOCK_OK,
			GTK_RESPONSE_NONE, NULL);
	content_area = gtk_dialog_get_content_area(GTK_DIALOG (dialog));     //make putting something into the popup possible
	label = gtk_label_new(message);     //make an error message

	g_signal_connect_swapped(dialog, "response",
			G_CALLBACK (gtk_widget_destroy), dialog);
	//make sure the popup is destroyed

	gtk_container_add(GTK_CONTAINER (content_area), label);     //put the message inside the popup
	gtk_widget_show_all(dialog);     //show what me make
}

void lan(GtkWidget *widget, gpointer data) {     //getting text then writing it on a button may be useful after overhauling

	gchar *buffer = data;

	if (send(sock, buffer, strlen(buffer), 0) != strlen(buffer))     //send a command
	puts("send() sent a different number of bytes than expected");

	rec = recv(sock, buffer, buffsize - 1, 0);     //get a message
	buffer[rec] = '\0';

	//printf("lan %s\n", buffer);

	gtk_button_set_label((GtkButton *) widget, buffer);     //show the message

}

void clcon(GtkWidget *widget, gpointer data) {     //connect or disconnect
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
		pixbuf = gdk_pixbuf_new_from_file_at_scale_utf8("pic/0.jpg", 500, 500,
				TRUE, error);
		gtk_image_set_from_pixbuf((GtkImage *) image, pixbuf);
	}
	return;
}

void login(GtkWidget *widget, gpointer data) {     //login function
	const gchar *login, *l_pass;
	char buffer[20];
	gchar full[64], temp[20];

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (status))) {     //see if we're connected PS:Not the best way is used

		login = gtk_entry_get_text((GtkEntry *) logname);     //get name from input
		l_pass = gtk_entry_get_text((GtkEntry *) pass);     //get password from input

		send(sock, login, strlen(login) + 1, 0);     //send name to the server
		send(sock, l_pass, strlen(l_pass) + 1, 0);     //send password to the server

		rec = recv(sock, buffer, buffsize - 1, 0);     //receive answer

		if (strcmp(buffer, "TRUE")) {     //see if we were successful or not
			return;
		} else {
			recv(sock, (char *) &user, sizeof(caracter), 0);
			printf(
					"%s ;%c ;%d.%d.%d ;%d ;%d ;%s ;%d ;%d ;%d ;%d ;%d ;%d ;%d ;%d ;%d ;"
							" Talents: ; Abilities: ; Techniques: ; Personality Traits: ; Reputation: ; Known Locations: ;",
					user.name, user.Gender, user.born.year, user.born.month,
					user.born.day, user.height, user.weight, user.House,
					user.str, user.dex, user.inte, user.vit, user.wis, user.sta,
					user.spd, user.cha, user.hly);

			g_stpcpy(full, user.House);
			g_strlcat(full, " ", 32);
			g_strlcat(full, user.name, 63);
			gtk_label_set_text((GtkLabel *) name, full);
			gtk_label_set_text((GtkLabel *) name, full);
			itoa(user.born.year, temp, 10);
			g_stpcpy(full, temp);
			g_strlcat(full, ".", 5);
			itoa(user.born.month, temp, 10);
			g_strlcat(full, temp, 7);
			g_strlcat(full, ".", 8);
			itoa(user.born.day, temp, 10);
			g_strlcat(full, temp, 10);
			gtk_label_set_text((GtkLabel *) born, full);
			gtk_label_set_text((GtkLabel *) gender,
					(user.Gender == 'F') ? "Female" : "Male");
			itoa(user.height, full, 10);
			gtk_label_set_text((GtkLabel *) height, full);
			itoa(user.weight, full, 10);
			gtk_label_set_text((GtkLabel *) weight, full);
			gtk_button_clicked((GtkButton *) cbutton);     //get some text
			gtk_widget_hide_all(logbox);     //exchange login fields with something else
			gtk_widget_show_all(gamebox);
			return;
		}
	} else {
		err("Please connect to the server");     //if we're not connected make an error message
	}
}

void reg() {     //register function
	GtkWidget *reg_dial, *label, *content_area, *r_name, *r_pass, *r_repass,
			*hbox;
	gint result;
	const gchar *login, *l_pass, *l_repass;
	gchar buffer[32];

	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (status))) {     //see if we're connected make an error message if not
		err("Please connect to the server");
		return;
	}
	//make the registration form
	reg_dial = gtk_dialog_new_with_buttons("Registration Form",
			(GtkWindow *) window,
			GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL, GTK_STOCK_OK,
			GTK_RESPONSE_OK, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL);
	content_area = gtk_dialog_get_content_area(GTK_DIALOG (reg_dial));
	label = gtk_label_new("Login:");
	r_name = gtk_entry_new();
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX (hbox), label, TRUE, FALSE, 10);
	gtk_box_pack_start(GTK_BOX (hbox), r_name, FALSE, FALSE, 10);
	gtk_container_add(GTK_CONTAINER (content_area), hbox);
	label = gtk_label_new("Pass:");
	r_pass = gtk_entry_new();
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX (hbox), label, TRUE, FALSE, 10);
	gtk_box_pack_start(GTK_BOX (hbox), r_pass, FALSE, FALSE, 10);
	gtk_container_add(GTK_CONTAINER (content_area), hbox);
	label = gtk_label_new("RePass:");
	r_repass = gtk_entry_new();
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX (hbox), label, TRUE, FALSE, 10);
	gtk_box_pack_start(GTK_BOX (hbox), r_repass, FALSE, FALSE, 10);
	gtk_container_add(GTK_CONTAINER (content_area), hbox);
	gtk_widget_show_all(reg_dial);

	send(sock, "reg", 4, 0);     //tell the server we want to register

	do {
		result = gtk_dialog_run(GTK_DIALOG (reg_dial));
		if (result != GTK_RESPONSE_OK) {
			gtk_widget_destroy(reg_dial);
			return;
		}
		login = gtk_entry_get_text((GtkEntry *) r_name);
		l_pass = gtk_entry_get_text((GtkEntry *) r_pass);
		l_repass = gtk_entry_get_text((GtkEntry *) r_repass);
	} while (g_strcmp0(l_pass, l_repass));

	send(sock, login, strlen(login) + 1, 0);     //send name to the server
	send(sock, l_pass, strlen(l_pass) + 1, 0);     //send password to the server

	recv(sock, buffer, buffsize - 1, 0);     //receive answer
	//g_printf("%s", buffer);

	if (g_strcmp0(buffer, "TRUE"))     //see if we were successful if not make an error message
		err(
				"There was a problem for security purposes start the registration from scratch");

	gtk_widget_destroy(reg_dial);     //destroy registration form
}

void set_image(GtkWidget *widget, gpointer data) {     //function for changing images
	GdkPixbuf *pixbuf;
	gchar *buffer = data;
	gchar src[10];

	send(sock, buffer, strlen(buffer), 0);     //send command

	rec = recv(sock, src, buffsize - 1, 0);     //receive path of the image
	src[rec] = '\0';

	//printf("image %s\n", src);

	pixbuf = gdk_pixbuf_new_from_file_at_scale_utf8(src, 500, 500, TRUE, error);     //read new image
	gtk_image_set_from_pixbuf((GtkImage *) image, pixbuf);     //exchange the old for the new image

}

int main(int argc, char *argv[]) {
	GtkWidget *box, *button, *poz, *reg_b, *frame;
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
	server.sin_addr.s_addr = inet_addr("127.0.0.1"); /* Server IP address */
	server.sin_port = htons(21); /* Server port */
	if (connect(sock, (struct sockaddr *) &server, sizeof(server)) < 0) /* Establish the connection to the server */
	puts("connect() failed");

	gtk_init(&argc, &argv);
	//start GTK+

	pixbuf = gdk_pixbuf_new_from_file_utf8("bgm.jpg", error);     //read background
	gdk_pixbuf_render_pixmap_and_mask(pixbuf, &bg, NULL, 0);
	style = gtk_style_new();
	style->bg_pixmap[0] = bg;

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);     //create window
	//gtk_window_set_default_size(GTK_WINDOW(window), 1280, 720);
	pixbuf = gdk_pixbuf_new_from_file_utf8("logo.gif", error);     //read logo
	gtk_window_set_icon(GTK_WINDOW (window), pixbuf);     //set logo
	gtk_container_set_border_width(GTK_CONTAINER (window), 10);
	//create close window event
	g_signal_connect(window, "delete-event", G_CALLBACK (delete_event), NULL);
	gtk_widget_set_style(GTK_WIDGET(window), GTK_STYLE(style));     //set background
	box = gtk_vbox_new(FALSE, 0);     //the mess of positioning
	poz = gtk_hbox_new(FALSE, 0);
	logbox = gtk_vbox_new(FALSE, 0);
	gamebox = gtk_vbox_new(FALSE, 0);     //positioning ends here

	//Setting up used Widgets -> buttons, input fields, ...
	//image = gtk_image_new_from_file_utf8("maemo.png");
	pixbuf = gdk_pixbuf_new_from_file_at_scale_utf8("pic/0.jpg", 500, 500, TRUE,     //read shown image
			error);
	image = gtk_image_new_from_pixbuf(pixbuf);     //set image
	frame = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type((GtkFrame *) frame, GTK_SHADOW_ETCHED_IN);
	gtk_container_add(GTK_CONTAINER (frame), image);
	gtk_container_add(GTK_CONTAINER (poz), frame);
	gtk_widget_show(frame);

	gtk_box_pack_start(GTK_BOX (poz), box, FALSE, FALSE, 10);     //a bit of positioning
	gtk_box_pack_start(GTK_BOX (box), logbox, FALSE, FALSE, 10);
	gtk_box_pack_start(GTK_BOX (box), gamebox, FALSE, FALSE, 10);     //end

	logname = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX (logbox), logname, FALSE, FALSE, 10);
	gtk_widget_show(logname);

	pass = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX (logbox), pass, FALSE, FALSE, 10);
	gtk_widget_show(pass);

	button = gtk_button_new_with_label("login");
	g_signal_connect(button, "clicked", G_CALLBACK (login), (gpointer) NULL);
	gtk_box_pack_start(GTK_BOX (logbox), button, FALSE, FALSE, 10);
	gtk_widget_show(button);

	reg_b = gtk_button_new_with_label("Register");
	g_signal_connect(reg_b, "clicked", G_CALLBACK (reg), (gpointer) "NULL");
	gtk_box_pack_start(GTK_BOX (logbox), reg_b, FALSE, FALSE, 10);
	gtk_widget_show(reg_b);

	cbutton = gtk_button_new_with_label(buffer);
	g_signal_connect(cbutton, "clicked", G_CALLBACK (lan), (gpointer) buffer);
	gtk_box_pack_start(GTK_BOX (gamebox), cbutton, FALSE, FALSE, 10);

	button = gtk_button_new_with_label("picture");
	g_signal_connect(button, "clicked", G_CALLBACK (set_image), "pic");
	gtk_box_pack_start(GTK_BOX (gamebox), button, FALSE, FALSE, 10);

	stats = make_stats();
	gtk_box_pack_start(GTK_BOX (gamebox), stats, FALSE, FALSE, 10);

	status = gtk_toggle_button_new_with_label("connected");
	gtk_toggle_button_set_active((GtkToggleButton *) status, TRUE);
	g_signal_connect(status, "toggled", G_CALLBACK (clcon), (gpointer) NULL);
	gtk_box_pack_start(GTK_BOX (box), status, FALSE, FALSE, 10);
	gtk_container_add(GTK_CONTAINER (window), poz);

	//showing the parts of the window
	gtk_widget_show(status);
	gtk_widget_show(image);
	gtk_widget_show(logbox);
	gtk_widget_show(poz);
	gtk_widget_show(box);
	gtk_widget_show(window);
	gtk_main();

	return EXIT_SUCCESS;
}
