#[macro_use]
pub mod util;
#[macro_use]
pub mod recipe;
#[macro_use]
pub mod config_util;
pub mod access;
pub mod action;
pub mod config;
pub mod factory;
pub mod item;
pub mod lua_value;
pub mod process;
pub mod server;
pub mod side;
pub mod storage;

use config::build_factory;
use crossterm::{
    event::{Event, EventStream},
    terminal::{disable_raw_mode, enable_raw_mode, EnterAlternateScreen, LeaveAlternateScreen},
    ExecutableCommand,
};
use futures_util::StreamExt;
use ratatui::{
    backend::CrosstermBackend,
    layout::{Constraint, Layout, Margin},
    style::Color,
    text::Line,
    widgets::{List, ListItem, Paragraph, Scrollbar, ScrollbarOrientation, ScrollbarState},
    Frame, Terminal,
};
use std::{
    cell::{Cell, RefCell},
    collections::VecDeque,
    io::stdout,
    rc::Rc,
};
use tokio::{select, sync::Notify, task::LocalSet};
use tui_textarea::{CursorMove, Input, Key, TextArea};

#[derive(Default)]
pub struct Tui {
    on_redraw: Notify,
    on_input: Notify,
    logs: RefCell<VecDeque<Line<'static>>>,
    input_queue: RefCell<Vec<String>>,
    text_area: RefCell<TextArea<'static>>,
    main_list: RefCell<Vec<Line<'static>>>,
    main_scroll: Cell<u16>,
    main_scroll_state: RefCell<ScrollbarState>,
}

impl Tui {
    fn request_redraw(&self) { self.on_redraw.notify_one() }
    fn log(&self, msg: String, color: u32) {
        let color = match color {
            0xFFFFFF => Color::Reset,
            0xFFA500 => Color::LightYellow,
            0x55ABEC => Color::LightBlue,
            0xF2B2CC => Color::LightRed,
            0xFF4FFF => Color::LightMagenta,
            0x00FF00 => Color::Green,
            0xFF0000 => Color::Red,
            _ => unreachable!(),
        };
        self.logs.borrow_mut().push_back(Line::styled(msg, color));
        self.request_redraw()
    }

    fn set_main_list(&self, list: Vec<Line<'static>>) {
        *self.main_list.borrow_mut() = list;
        self.set_main_scroll(|x| x)
    }

    fn set_main_scroll(&self, upd: impl FnOnce(u16) -> u16) {
        let list = self.main_list.borrow();
        let i = upd(self.main_scroll.get());
        self.main_scroll.set(i.min(list.len().max(1) as u16 - 1));
        let mut state = self.main_scroll_state.borrow_mut();
        *state = state.position(i as _).content_length(list.len())
    }

    fn frame(&self, frame: &mut Frame) {
        let layout = Layout::vertical([Constraint::Fill(1), Constraint::Length(1)]).split(frame.size());
        frame.render_widget(self.text_area.borrow().widget(), layout[1]);

        let log_size;
        let main_list = self.main_list.borrow();
        if main_list.is_empty() {
            log_size = layout[0]
        } else {
            let layout = Layout::horizontal([Constraint::Percentage(50), Constraint::Fill(1)]).split(layout[0]);
            log_size = layout[0];
            let main_list_size = layout[1];
            frame.render_widget(Paragraph::new(main_list.clone()).scroll((self.main_scroll.get(), 0)), main_list_size);
            let scroll = Scrollbar::new(ScrollbarOrientation::VerticalRight);
            frame.render_stateful_widget(
                scroll,
                main_list_size.inner(Margin { horizontal: 1, vertical: 0 }),
                &mut *self.main_scroll_state.borrow_mut(),
            )
        }

        let mut log_buffer = self.logs.borrow_mut();
        while log_buffer.len() > log_size.height as _ {
            log_buffer.pop_front();
        }
        frame.render_widget(Paragraph::new(Vec::from_iter(log_buffer.iter().cloned())), log_size)
    }
}

#[tokio::main(flavor = "current_thread")]
async fn main() {
    let tasks = LocalSet::new();
    tasks.spawn_local(async {
        enable_raw_mode().unwrap();
        stdout().execute(EnterAlternateScreen).unwrap();
        let mut evts = EventStream::new();
        let mut term = Terminal::new(CrosstermBackend::new(std::io::stderr())).unwrap();
        let tui = Rc::<Tui>::default();
        let _factory = build_factory(tui.clone());
        loop {
            term.draw(|frame| tui.frame(frame)).unwrap();
            let evt = select! {
                () = tui.on_redraw.notified() => None,
                evt = evts.next() => if let Some(Ok(x)) = evt { Some(x) } else { break }
            };
            if let Some(Event::Key(evt)) = evt {
                let evt = Input::from(evt);
                if evt.ctrl && (evt.key == Key::Char('c') || evt.key == Key::Char('d')) {
                    break;
                } else if evt.ctrl && evt.key == Key::Char('l') {
                    tui.logs.borrow_mut().clear()
                } else if evt.key == Key::PageUp {
                    tui.set_main_scroll(|x| x.saturating_sub(8))
                } else if evt.key == Key::PageDown {
                    tui.set_main_scroll(|x| x.saturating_add(8))
                } else if evt.ctrl && evt.key == Key::Char('m') || evt.key == Key::Enter {
                    let mut text_area = tui.text_area.borrow_mut();
                    tui.input_queue.borrow_mut().extend(text_area.lines().get(text_area.cursor().0).cloned());
                    text_area.move_cursor(CursorMove::End);
                    text_area.insert_newline()
                } else {
                    tui.text_area.borrow_mut().input(evt);
                }
                tui.on_input.notify_waiters()
            }
        }
        disable_raw_mode().unwrap();
        stdout().execute(LeaveAlternateScreen).unwrap();
    });
    tasks.await
}
