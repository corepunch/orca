// Weather's home-screen and lock-screen widget (PLAN.md N3). It only reads
// widget.json, which the app (apps/Weather/widget.lua) writes into the App
// Group named by this extension's ORCAAppGroup Info.plist key; it never
// fetches. tools/ios/widget.py builds it into the app with WIDGETS=1.
import SwiftUI
import WidgetKit

struct Snapshot: Decodable {
    var version: Int
    var app: String
    var place: String
    var temp: String
    var sky: String
    var activity: String
    var verdict: String
    var tone: String
    var line: String
    var updated: Double

    static let sample = Snapshot(version: 1, app: "Weather", place: "your place", temp: "12°", sky: "partly cloudy",
                                 activity: "outdoors", verdict: "good", tone: "good", line: "outdoors best 10–16h",
                                 updated: Date().timeIntervalSince1970)

    static func load() -> Snapshot? {
        guard let group = Bundle.main.object(forInfoDictionaryKey: "ORCAAppGroup") as? String,
              let folder = FileManager.default.containerURL(forSecurityApplicationGroupIdentifier: group),
              let data = try? Data(contentsOf: folder.appendingPathComponent("widget.json"))
        else { return nil }
        return try? JSONDecoder().decode(Snapshot.self, from: data)
    }
}

struct SnapshotEntry: TimelineEntry {
    var date: Date
    var snapshot: Snapshot?
}

struct SnapshotProvider: TimelineProvider {
    func placeholder(in context: Context) -> SnapshotEntry { SnapshotEntry(date: Date(), snapshot: .sample) }

    func getSnapshot(in context: Context, completion: @escaping (SnapshotEntry) -> Void) {
        completion(SnapshotEntry(date: Date(), snapshot: Snapshot.load() ?? (context.isPreview ? .sample : nil)))
    }

    // The app writes when it draws its home place and after a background
    // refresh; reading again every half hour picks that up.
    func getTimeline(in context: Context, completion: @escaping (Timeline<SnapshotEntry>) -> Void) {
        let now = Date()
        completion(Timeline(entries: [SnapshotEntry(date: now, snapshot: Snapshot.load())],
                            policy: .after(now.addingTimeInterval(30 * 60))))
    }
}

// The app's verdict colours: green, amber, red; white for information.
private func toneColor(_ tone: String) -> Color {
    switch tone {
    case "good": return Color(red: 0.38, green: 0.75, blue: 0.29)
    case "fair": return Color(red: 0.94, green: 0.64, blue: 0.04)
    case "poor": return Color(red: 0.90, green: 0.08, blue: 0.0)
    default: return .white
    }
}

private func age(_ snapshot: Snapshot, at date: Date) -> String {
    let minutes = Int(date.timeIntervalSince1970 - snapshot.updated) / 60
    if minutes < 2 { return "updated just now" }
    if minutes < 60 { return "updated \(minutes) min ago" }
    return "updated \(minutes / 60) h ago"
}

struct WeatherWidgetView: View {
    @Environment(\.widgetFamily) private var family
    var entry: SnapshotEntry

    var body: some View {
        content.widgetBackground()
    }

    @ViewBuilder private var content: some View {
        if let snapshot = entry.snapshot {
            switch family {
            case .accessoryInline:
                Text("\(snapshot.temp) \(snapshot.activity) \(snapshot.verdict)")
            case .accessoryRectangular:
                VStack(alignment: .leading, spacing: 0) {
                    Text("\(snapshot.place) \(snapshot.temp)").font(.headline).lineLimit(1)
                    Text("\(snapshot.activity): \(snapshot.verdict)").lineLimit(1)
                    Text(snapshot.line).font(.caption).lineLimit(1)
                }
                .accessibilityElement(children: .combine)
            default:
                VStack(alignment: .leading, spacing: 2) {
                    Text(snapshot.place.lowercased()).font(.system(size: 15)).foregroundColor(.white).lineLimit(1)
                    Text(snapshot.temp).font(.system(size: 44, weight: .light)).foregroundColor(.white)
                    Text(snapshot.verdict).font(.system(size: 20)).foregroundColor(toneColor(snapshot.tone)).lineLimit(1)
                    Text(snapshot.line).font(.system(size: 12)).foregroundColor(.gray).lineLimit(family == .systemSmall ? 2 : 1)
                    Spacer(minLength: 0)
                    Text(age(snapshot, at: entry.date)).font(.system(size: 10)).foregroundColor(.gray)
                }
                .frame(maxWidth: .infinity, maxHeight: .infinity, alignment: .topLeading)
                .accessibilityElement(children: .combine)
            }
        } else {
            Text("Open the app to choose a home place.").font(.system(size: 13)).foregroundColor(.gray)
        }
    }
}

private extension View {
    // A black tile, as the app; iOS 17 asks for it through containerBackground.
    @ViewBuilder func widgetBackground() -> some View {
        if #available(iOSApplicationExtension 17.0, *) {
            containerBackground(Color.black, for: .widget)
        } else {
            padding().background(Color.black)
        }
    }
}

struct WeatherWidget: Widget {
    var body: some WidgetConfiguration {
        StaticConfiguration(kind: "WeatherWidget", provider: SnapshotProvider()) { entry in
            WeatherWidgetView(entry: entry)
        }
        .configurationDisplayName(Bundle.main.object(forInfoDictionaryKey: "CFBundleDisplayName") as? String ?? "Weather")
        .description("Your home place: the temperature and whether now is good.")
        .supportedFamilies([.systemSmall, .systemMedium, .accessoryRectangular, .accessoryInline])
    }
}

@main
struct WeatherWidgets: WidgetBundle {
    var body: some Widget { WeatherWidget() }
}
