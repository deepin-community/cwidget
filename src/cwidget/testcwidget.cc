// testcwidget.cc
//
//  Not pretty code, but it tests some of the facilities of cwidget.

#include <cwidget/dialogs.h>
#include <cwidget/fragment.h>
#include <cwidget/config/colors.h>
#include <cwidget/generic/util/transcode.h>
#include <cwidget/generic/util/exception.h>
#include <cwidget/toplevel.h>
#include <cwidget/widgets/button.h>
#include <cwidget/widgets/center.h>
#include <cwidget/widgets/editline.h>
#include <cwidget/widgets/frame.h>
#include <cwidget/widgets/label.h>
#include <cwidget/widgets/layout_item.h>
#include <cwidget/widgets/menu.h>
#include <cwidget/widgets/menubar.h>
#include <cwidget/widgets/minibuf_win.h>
#include <cwidget/widgets/multiplex.h>
#include <cwidget/widgets/pager.h>
#include <cwidget/widgets/radiogroup.h>
#include <cwidget/widgets/scrollbar.h>
#include <cwidget/widgets/size_box.h>
#include <cwidget/widgets/stacked.h>
#include <cwidget/widgets/subtree.h>
#include <cwidget/widgets/table.h>
#include <cwidget/widgets/text_layout.h>
#include <cwidget/widgets/togglebutton.h>
#include <cwidget/widgets/tree.h>
#include <cwidget/widgets/widget.h>

#include <sigc++/adaptors/bind.h>
#include <sigc++/functors/mem_fun.h>
#include <sigc++/functors/ptr_fun.h>

#include <iostream>
#include <string>


using namespace std;

using namespace cwidget;
using namespace cwidget::widgets;

// Er..having this be global is not so great?
stacked_ref stacker=stacked::create(0,0);

class test_keyname:public label
{
public:
  test_keyname():label("")
  {
    set_bg_style(get_style("EditLine"));
  }

  widgets::size size_request()
  {
    return widgets::size(10, 0);
  }

  bool focus_me() {return true;}

  bool handle_key(const config::key &k)
  {
    set_text(config::readable_keyname(k));
    return true;
  }
};

class silly_block:public widget
{
public:
  silly_block(const style &st)
  {
    set_bg_style(st);
  }

  widgets::size size_request()
  {
    return widgets::size(0,0);
  }

  void paint()
  {
  }
};

class silly_treeitem:public treeitem
{
  wstring txt;
public:
  silly_treeitem(const wstring &_txt):txt(_txt) {}
  silly_treeitem(const string &_txt):txt(util::transcode(_txt)) {}

  void paint(tree *win, int y, bool hierarchical, const style &st)
  {
    treeitem::paint(win, y, hierarchical, txt);
  }

  const wchar_t *tag() {return txt.c_str();}
  const wchar_t *label() {return txt.c_str();}
};

class silly_subtree:public subtree_generic
{
  wstring txt;
public:
  silly_subtree(bool expanded, const wstring &_txt)
    :subtree_generic(expanded), txt(_txt) {}

  silly_subtree(bool expanded, const string &_txt)
    :subtree_generic(expanded), txt(util::transcode(_txt)) {}

  void paint(tree *win, int y, bool hierarchical, const style &st)
  {
    subtree_generic::paint(win, y, hierarchical, txt);
  }

  const wchar_t *tag() {return txt.c_str();}
  const wchar_t *label() {return txt.c_str();}
};

void do_toggle_hierarchical(tree &tree)
{
  tree.set_hierarchical(!tree.get_hierarchical());
}

widget_ref make_test_treewidget()
{
  tree_ref tree=tree::create();

  silly_subtree *root=new silly_subtree(true, "The Root Of It All");
  silly_subtree *tree1=new silly_subtree(false, "Money");
  tree1->add_child(new silly_treeitem("Gold"));
  tree1->add_child(new silly_treeitem("Copper"));
  tree1->add_child(new silly_treeitem("Silver"));
  tree1->add_child(new silly_treeitem("Paper"));
  root->add_child(tree1);

  silly_subtree *tree2=new silly_subtree(false, "Entropy");
  silly_subtree *tree3=new silly_subtree(true, "Mortality");
  tree3->add_child(new silly_treeitem("Death"));
  tree3->add_child(new silly_treeitem("Famine"));
  tree3->add_child(new silly_treeitem("Pestilence"));
  tree3->add_child(new silly_treeitem("War"));
  tree2->add_child(tree3);
  tree2->add_child(new silly_treeitem("Rot"));
  tree2->add_child(new silly_treeitem("Decay"));
  tree2->add_child(new silly_treeitem("Bad Things"));
  root->add_child(tree2);

  // A commentary on the analytical machine, translated by Lady Ada.
  silly_subtree *anaengine=new silly_subtree(false, "Charles B.");
  anaengine->add_child(new layout_item(sequence_fragment(flowbox(text_fragment("Those labours which belong to the various branches of the mathematical sciences, although on first consideration they seem to be the exclusive province of intellect, may, nevertheless, be divided into two distinct sections; one of which may be called the mechanical, because it is subjected to precise and invariable laws, that are capable of being expressed by means of the operations of matter; while the other, demanding the intervention of reasoning, belongs more specially to the domain of the understanding. This admitted, we may propose to execute, by means of machinery, the mechanical branch of these labours, reserving for pure intellect that which depends on the reasoning faculties. Thus the rigid exactness of those laws which regulate numerical calculations must frequently have suggested the employment of material instruments, either for executing the whole of such calculations or for abridging them; and thence have arisen several inventions having this object in view, but which have in general but partially attained it. For instance, the much-admired machine of Pascal is now simply an object of curiosity, which, whilst it displays the powerful intellect of its inventor, is yet of little utility in itself. Its powers extended no further than the execution of the first four operations of arithmetic, and indeed were in reality confined to that of the first two, since multiplication and division were the result of a series of additions and subtractions. The chief drawback hitherto on most of such machines is, that they require the continual intervention of a human agent to regulate their movements, and thence arises a source of errors; so that, if their use has not become general for large numerical calculations, it is because they have not in fact resolved the double problem which the question presents, that of correctness in the results, united with economy of time.")),
							    newline_fragment(),
							    flowbox(text_fragment("Struck with similar reflections, Mr. Babbage has devoted some years to the realization of a gigantic idea. He proposed to himself nothing less than the construction of a machine capable of executing not merely arithmetical calculations, but even all those of analysis, if their laws are known. The imagination is at first astounded at the idea of such an undertaking; but the more calm reflection we bestow on it, the less impossible does success appear, and it is felt that it may depend on the discovery of some principle so general, that, if applied to machinery, the latter may be capable of mechanically translating the operations which may be indicated to it by algebraical notation. The illustrious inventor having been kind enough to communicate to me some of his views on this subject during a visit he made at Turin, I have, with his approbation, thrown together the impressions they have left on my mind. But the reader must not expect to find a description of Mr. Babbage's engine; the comprehension of this would entail studies of much length; and I shall endeavour merely to give an insight into the end proposed, and to develop the principles on which its attainment depends.")),
							    NULL)));
  root->add_child(anaengine);

  silly_subtree *tree4=new silly_subtree(false, "More stuff");
  for(int i=0; i<10; i++)
    {
      char buf[32];

      snprintf(buf, sizeof(buf), "Category %d", i);

      silly_subtree *sub=new silly_subtree(false, buf);
      for(int j=0; j<10; j++)
	{
	  snprintf(buf, sizeof(buf), "Item %d", j);
	  sub->add_child(new silly_treeitem(buf));
	}

      tree4->add_child(sub);
    }
  root->add_child(tree4);
  root->sort();

  tree->set_root(root, true);

  tree->connect_key("ToggleHier", &config::global_bindings,
		    sigc::bind(sigc::ptr_fun(do_toggle_hierarchical),
			       tree.weak_ref()));

  return tree;
}

void show_nasty_message()
{
  stacker->add_visible_widget(dialogs::ok(flowbox(fragf("Your mother was a hamster, and your father smelt of elderberry!%n%nNow go away, or I shall taunt you a second time!")), NULL), true);
}

void interrogate()
{
  stacker->add_visible_widget(dialogs::yesno(flowbox(fragf("Do you like Debian?")), NULL, util::arg(sigc::ptr_fun(show_nasty_message))), true);
}

void dobeep()
{
  beep();
}

menu_info test_file_menu[]=
{
  menu_info(menu_info::MENU_ITEM, "^Interrogate", NULL,
	    "NO-ONE EXPECTS THE SPANISH INQUISITION!", sigc::ptr_fun(&interrogate)),
  MENU_SEPARATOR,
  menu_info(menu_info::MENU_ITEM, "^Quit", "Quit",
	    "Leave this wonderful program", sigc::ptr_fun(&toplevel::exitmain)),
  MENU_END,
};

menu_info test_test_menu[]=
{
  menu_info(menu_info::MENU_ITEM, "Test ^Item 1", NULL, "Foo", sigc::ptr_fun(&dobeep)),
  menu_info(menu_info::MENU_ITEM, "Test Item ^2", NULL, "Bar", sigc::ptr_fun(&dobeep)),
  MENU_SEPARATOR,
  menu_info(menu_info::MENU_ITEM, "Test Item ^3", NULL, "Baz", sigc::ptr_fun(&dobeep)),
  MENU_END
};

menu_info test_help_menu[]=
{
  menu_info(menu_info::MENU_ITEM, "^About", NULL, "Useless dialog box",
	    MENU_NOP),
  MENU_SEPARATOR,
  menu_info(menu_info::MENU_ITEM, "^Help...", "Help", "unimplemented",
	    MENU_NOP),
  menu_info(menu_info::MENU_ITEM, "^Don't help...", NULL, "unimplemented",
	    MENU_NOP),
  MENU_END
};

void radio_button_selected(int id, label &l)
{
  char buf[128];

  snprintf(buf, 128, "You have selected button %i", id);
  l.set_text(buf);
}

widget_ref button_mania()
{
  table_ref rval=table::create();
  label_ref label=label::create("ERROR");

  rval->add_widget(label, 5, 0, 1, 4, false);

  for(int i=0; i<4; i++)
    {
      radiogroup *g=NULL;

      if(i==3)
	{
	  g=new radiogroup;
	  g->item_selected.connect(sigc::bind(sigc::ptr_fun(&radio_button_selected),
					      label.weak_ref()));
	  rval->destroyed.connect(sigc::mem_fun(g, &radiogroup::destroy));
	}
      for(int j=0; j<4; j++)
	{
	  char buf[256];
	  snprintf(buf, 256, "%i", i*4+j);
	  fragment *f=fragf("Button %B%s%b", buf);
	  if(j==2)
	    f=fragf("Line 1%n%F%nLine 3", f);

	  if(i!=3)
	    rval->add_widget_opts(button::create(f), i, j, 1, 1,
				  table::ALIGN_CENTER, table::ALIGN_CENTER);
	  else
	    {
	      togglebutton_ref b=radiobutton::create(f);
	      rval->add_widget_opts(b, i, j, 1, 1,
				    table::ALIGN_CENTER | table::EXPAND, table::ALIGN_CENTER);
	      g->add_button(b, i*4+j);
	    }
	}
    }

  rval->add_widget_opts(checkbutton::create("Button 16"), 4, 0, 1, 2,
			table::ALIGN_CENTER,
			table::ALIGN_CENTER | table::EXPAND);
  rval->add_widget_opts(button::create("Button 17"), 4, 2, 1, 2,
			table::ALIGN_CENTER,
			table::ALIGN_CENTER | table::EXPAND);

  return rval;
}

static void do_load_file(wstring s, editline &p)
{
  p.set_text(L"");
}

widget_ref pager_test()
{
  table_ref tbl=table::create();
  editline_ref ln=editline::create("Enter a filename to load: ");
  const char *s="This space for rent.\n\nApply above.";
  file_pager_ref pager=file_pager::create(s, strlen(s));
  scrollbar_ref scrl=scrollbar::create(scrollbar::VERTICAL, 0, 0);

  tbl->add_widget_opts(ln, 0, 0, 1, 2, table::EXPAND | table::FILL | table::SHRINK, table::SHRINK);
  tbl->add_widget_opts(pager, 1, 0, 1, 1, table::SHRINK, table::EXPAND | table::FILL);
  tbl->add_widget_opts(scrl, 1, 1, 1, 1, table::ALIGN_LEFT, table::EXPAND | table::FILL);

  ln->entered.connect(sigc::bind(sigc::ptr_fun(&do_load_file), ln.weak_ref()));
  ln->entered.connect(sigc::mem_fun(*pager.unsafe_get_ref(), (void (file_pager::*)(const std::wstring &)) &file_pager::load_file));
  pager->line_changed.connect(sigc::mem_fun(*scrl.unsafe_get_ref(), &scrollbar::set_slider));
  pager->do_line_signal();
  scrl->scrollbar_interaction.connect(sigc::mem_fun(*pager.unsafe_get_ref(), &pager::scroll_page));

  return tbl;
}

void update_menu_status(const menu_item *item, label &label)
{
  if(item)
    {
      label.show();
      label.set_text(item->get_description(),
		     get_style("Status"));
    }
  else
    label.hide();
}

void do_editline_history(std::wstring s, editline &l)
{
  l.add_to_history(s);
  l.reset_history();
  l.set_text("");
}

fragment *dickens_fragment()
{

  vector<fragment*> v;

  v.push_back(indentbox(2, 2, flowbox(fragf("%s %F %s", "A", style_fragment(text_fragment("Christmas"), get_style("Error")), "Carol"))));

  v.push_back(newline_fragment());

  v.push_back(flowbox(fragf("%BMarley was %s%b: to begin with.  There is %F whatever about that. The register of his burial was signed by the clergyman, the clerk, the undertaker, and the chief mourner. Scrooge signed it. And Scrooge's name was good upon 'Change, for anything he chose to put his hand to. Old Marley was as dead as a door-nail.",
			    "dead",
			    text_fragment("no doubt", style_attrs_on(A_BOLD)))));

  v.push_back(newline_fragment());

  // Test positional arguments.
  v.push_back(fillbox(fragf("%2$F! I don't mean to say that I know, of my own knowledge, what there is particularly dead about a door-nail. I might have been inclined, myself, to regard a coffin-nail as the deadest piece of ironmongery in the trade. But the wisdom of our ancestors is in the simile; and my unhallowed hands shall not disturb it, or the Country's done for. You will therefore permit me to repeat, emphatically, that Marley was as %3$F as a %1$F.%n%n%n%nScrooge knew he was dead? Of course he did. How could it be otherwise? Scrooge and he were partners for I don't know how many years. Scrooge was his sole executor, his sole administrator, his sole assign, his sole residuary legatee, his sole friend, and sole mourner. And even Scrooge was not so dreadfully cut up by the sad event, but that he was an excellent man of business on the very day of the funeral, and solemnised it with an undoubted bargain.",
			    text_fragment("door-nail", style_attrs_on(A_BOLD)),
			    style_fragment(text_fragment("Mind"), style_attrs_on(A_BOLD)),
			    text_fragment("dead", style_attrs_on(A_BOLD)))));

  v.push_back(newline_fragment());

  v.push_back(hardwrapbox(fragf("The mention of Marley's funeral brings me back to the point I started from.  There is no doubt that Marley was dead.  This must be distinctly understood, or nothing wonderful can come of the story I am going to relate.  If we were not perfectly convinced that Hamlet's Father died before the play began, there would be nothing more remarkable in his taking a stroll at night, in an easterly wind, upon his own ramparts, than there would be in any other middle-aged gentleman rashly turning out after dark in a breezy spot -- say Saint Paul's Churchyard for instance -- literally to astonish his son's weak mind.")));

  v.push_back(newline_fragment());

  vector<fragment_column_entry> column_entries;

  std::vector<fragment *> column1_lines;
  column1_lines.push_back(flowbox(fragf("Scrooge never painted out Old Marley's name. There it stood, years afterwards, above the warehouse door: Scrooge and Marley.  The firm was known as Scrooge and Marley.  Sometimes people new to the business called Scrooge Scrooge, and sometimes Marley, but he answered to both names: it was all the same to him.\n\nOh!  But he was a tight-fisted hand at the grind-stone, Scrooge! a squeezing, wrenching, grasping, scraping, clutching, covetous, old sinner!  Hard and sharp as flint, from which no steel had ever struck out generous fire; secret, and self-contained, and solitary as an oyster.  The cold within him froze his old features, nipped his pointed nose, shriveled his cheek, stiffened his gait; made his eyes red, his thin lips blue and spoke out shrewdly in his grating voice.  A frosty rime was on his head, and on his eyebrows, and his wiry chin.  He carried his own low temperature always about with him; he iced his office in the dogdays; and didn't thaw it one degree at Christmas.")));

  column1_lines.push_back(newline_fragment());
  column1_lines.push_back(newline_fragment());
  column1_lines.push_back(flowbox(text_fragment("External heat and cold had little influence on Scrooge.  No warmth could warm, no wintry weather chill him.  No wind that blew was bitterer than he, no falling snow was more intent upon its purpose, no pelting rain less open to entreaty.  Foul weather didn't know where to have him.  The heaviest rain, and snow, and hail, and sleet, could boast of the advantage over him in only one respect.  They often \"came down\" handsomely, and Scrooge never did.")));

  column_entries.push_back(fragment_column_entry(true, false, 5, fragment_column_entry::top, column1_lines));
  column_entries.push_back(fragment_column_entry(false, false, 1, fragment_column_entry::top, std::vector<fragment *>()));

  std::vector<fragment *> column3_lines;
  column3_lines.push_back(flowbox(fragf("Here we see further evidence of Scrooge's miserly behavior: to save himself a bit of money on paint and labour, he leaves an inaccurate (and somewhat spooky) sign above the door of his business.")));
  column3_lines.push_back(newline_fragment());
  column3_lines.push_back(flowbox(fragf("Spoiler: things only get spookier from here on.")));

  column_entries.push_back(fragment_column_entry(true, false, 1, fragment_column_entry::bottom, column3_lines));

  v.push_back(fragment_columns(column_entries));

  return sequence_fragment(v);
}

// test wrapping around utf8.
fragment *chinese_fragment()
{
  vector<fragment *> v;

  v.push_back(clipbox(fragf("Here is some Chinese text:%n%n%ls", L"\u6211\u7684\u6c23\u588a\u8239\u5145\u6eff\u4e86\u9c54\u9b5a")));

  v.push_back(newline_fragment());

  v.push_back(flowbox(fragf("Here is some Chinese text:%n%n%ls", L"\u6211\u7684\u6c23\u588a\u8239\u5145\u6eff\u4e86\u9c54\u9b5a")));

  v.push_back(newline_fragment());

  v.push_back(hardwrapbox(fragf("Here is some Chinese text:%n%n%ls", L"\u6211\u7684\u6c23\u588a\u8239\u5145\u6eff\u4e86\u9c54\u9b5a")));

  v.push_back(newline_fragment());

  v.push_back(fillbox(fragf("Here is some Chinese text:%n%n%ls", L"\u6211\u7684\u6c23\u588a\u8239\u5145\u6eff\u4e86\u9c54\u9b5a")));

  return sequence_fragment(v);
}

// This isn't a very comprehensive test yet.
widget_ref make_layout_test(fragment *f)
{
  table_ref t=table::create();


  text_layout_ref l=text_layout::create(f);
  scrollbar_ref s=scrollbar::create(scrollbar::VERTICAL);
  l->location_changed.connect(sigc::mem_fun(*s.unsafe_get_ref(), &scrollbar::set_slider));
  s->scrollbar_interaction.connect(sigc::mem_fun(*l.unsafe_get_ref(), &text_layout::scroll));

  t->add_widget_opts(l, 0, 0, 1, 1, table::EXPAND | table::SHRINK, table::EXPAND);
  t->add_widget_opts(s, 0, 1, 1, 1, 0, table::EXPAND | table::FILL);

  return t;
}

int main(int argc, char *argv[])
{
  setlocale(LC_ALL, "");

  toplevel::init();

  config::global_bindings.set("CycleScreen", config::key(KEY_F(6), true));
  config::global_bindings.set("CycleScreenBack", config::key(KEY_F(7), true));
  config::global_bindings.set("ToggleCellVisible", config::key(KEY_F(5), true));
  config::global_bindings.set("ToggleHier", config::key(L'h', false));

  menubar_ref menubar=menubar::create();

  menubar->connect_key_post("Quit", &config::global_bindings, sigc::ptr_fun(&toplevel::exitmain));

  label_ref menu_display=label::create("", get_style("Status"));

  menu_ref menu(menu::create(0, 0, 0, test_file_menu));
  menubar->append_item(L"File", menu);
  menu->item_highlighted.connect(sigc::bind(sigc::ptr_fun(&update_menu_status),
					    menu_display.weak_ref()));

  menu=menu::create(0, 0, 0, test_test_menu);
  menubar->append_item(L"Test", menu);
  menu->item_highlighted.connect(sigc::bind(sigc::ptr_fun(&update_menu_status),
					    menu_display.weak_ref()));

  menu=menu::create(0, 0, 0, test_help_menu);
  menubar->append_item(L"Help", menu);
  menu->item_highlighted.connect(sigc::bind(sigc::ptr_fun(&update_menu_status),
					    menu_display.weak_ref()));

  minibuf_win_ref toplevel=minibuf_win::create();

  toplevel->add_widget(menu_display);

  menubar->set_subwidget(toplevel);

  multiplex_ref switcher=multiplex::create(true);
  switcher->connect_key("CycleScreen",
			&config::global_bindings,
			sigc::mem_fun(*switcher.unsafe_get_ref(), &multiplex::cycle_forward));
  switcher->connect_key("CycleScreenBack",
			&config::global_bindings,
			sigc::mem_fun(*switcher.unsafe_get_ref(), &multiplex::cycle_backward));

  switcher->add_visible_widget(pager_test(), true);
  switcher->add_visible_widget(button_mania(), true);

  switcher->add_visible_widget(center::create(size_box::create(widgets::size(20, 8), frame::create(center::create(label::create(flowbox(fragf("This is another screen.%nNotice that this label is properly word-wrapped."))))))), true);
  switcher->add_visible_widget(label::create("This is one screen."), true);
  switcher->add_visible_widget(dialogs::ok(util::transcode("Press any key to hide this widget")), true);
  switcher->add_visible_widget(make_test_treewidget(), true);

  table_ref ttable=table::create();
  ttable->add_widget(frame::create(label::create("Press a key:")));
  ttable->add_widget(frame::create(new test_keyname));
  ttable->show_all();
  switcher->add_widget(ttable);

  table_ref table=table::create();
  table->set_colsep(3);
  widget_ref w=frame::create(center::create(label::create("Pane 1")));
  table->add_widget(w, 0, 0, 2, 1, false);
  w=frame::create(center::create(label::create("Pane 2")));
  table->add_widget_opts(w, 0, 1, 1, 1, table::EXPAND | table::FILL, 0);

  editline::history_list h;
  editline_ref l=editline::create(20, "Input: ", "Pane 3", &h);
  l->set_allow_wrap(true);
  l->entered.connect(sigc::bind(sigc::ptr_fun(&do_editline_history),
				l.weak_ref()));

  w=frame::create(center::create(l));
  table->add_widget(w, 1, 1);
  w=frame::create(center::create(label::create("Pane 4 - a pane with a very long label in it\nwhich should crowd out the other table cells")));
  table->connect_key("ToggleCellVisible",
		     &config::global_bindings,
		     sigc::mem_fun(*w.unsafe_get_ref(), &widget::toggle_visible));
  table->add_widget(w, 0, 2, 2);
  table->show_all();
  switcher->add_widget(table);

  switcher->add_visible_widget(make_layout_test(chinese_fragment()), true);
  switcher->add_visible_widget(make_layout_test(dickens_fragment()), true);

  stacker->add_visible_widget(switcher, true);

  stacker->add_visible_widget(dialogs::ok(util::transcode("CWidget test program - press any key to begin")), true);

  toplevel->set_main_widget(stacker);
  stacker->show();
  toplevel->set_header("testcwidget");
  toplevel->set_header("This program tests the cwidget text UI library");

  toplevel::settoplevel(menubar);

  try
    {
      toplevel::mainloop();
    }
  catch(const std::exception &e)
    {
      toplevel::shutdown();
      std::cerr << "Caught exception: " << e.what() << std::endl;
      return 0;
    }
  catch(const util::Exception &e)
    {
      toplevel::shutdown();
      std::cerr << "Caught exception: " << e.errmsg() << std::endl;
      return 0;
    }
  catch(...)
    {
      toplevel::shutdown();
      std::cerr << "Uncaught exception." << std::endl;
      return 0;
    }

  toplevel::shutdown();

  return 0;
}
