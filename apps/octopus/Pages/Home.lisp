(Node2D Name="HomeContent"
  (Pivot Name="Timeline" Title="Octopus" ExtendBodyBehindHeader=true SelectionChanged="SelectFeed" class="octopus-ink"
    (PivotItem Name="Feedhome" Header="home"
      (Grid
        (StackView Name="Timelinehome" Spacing=12 Overflow="hidden scroll" JustifyContent="Start"
          (StackView Name="Guesthome" Visible=(bind "DataContext/Reader/IsGuest") Spacing=16 JustifyContent="Start"
            (StackView Spacing=8 Padding="0 0 24 0" JustifyContent="Start"
              (TextBlock Text="your timeline lives here" FontSize=22 WordWrap=true class="octopus-muted" IgnoreHitTest=true)
              (TextBlock Text="Sign in to follow people and see their posts and reposts in one place." FontSize=14 WordWrap=true class="octopus-dim" IgnoreHitTest=true))
            (Button Name="SignInhome" Click="SignIn" Height=48 FontSize=15 FontWeight="Bold" TextHorizontalAlignment="Center" TextVerticalAlignment="Center" class="octopus-action octopus-primary" "Sign in")
            (Button Name="SignUphome" Click="SignUp" Height=48 FontSize=15 TextHorizontalAlignment="Left" TextVerticalAlignment="Center" class="octopus-action octopus-link" "Create an account"))
          (ItemsStack Name="PostItems" Spacing=12 JustifyContent="Start" ItemNamePrefix="Post" ItemsSource=(bind "DataContext/Home/Items") ItemTemplate="Octopus/Templates/PostCard")
          (StackView Name="EmptyTimelinehome" Visible=(bind "DataContext/Home/ShowEmpty") Spacing=8 Padding="0 0 24 0" JustifyContent="Start"
            (TextBlock Text="your timeline is quiet" FontSize=22 WordWrap=true class="octopus-muted" IgnoreHitTest=true)
            (TextBlock Text="Follow people from discover or search to see their posts here." FontSize=14 WordWrap=true class="octopus-dim" IgnoreHitTest=true))
          (StackView Name="TimelineErrorhome" Visible=(bind "DataContext/Home/ShowError") Spacing=8 Padding="0 0 24 0" JustifyContent="Start"
            (TextBlock Text="can't load posts" FontSize=22 WordWrap=true class="octopus-muted" IgnoreHitTest=true)
            (TextBlock FontSize=14 WordWrap=true class="octopus-dim" IgnoreHitTest=true Text=(+ (bind "DataContext/Home/ErrorMessage") " Use refresh to try again.")))
          (Grid Name="LoadingMorehome" Visible=(bind "DataContext/Home/IsLoadingMore") Height=64 IgnoreHitTest=true
            (ActivityIndicator class="octopus-dim"))
          (Button Name="LoadMorehome" Visible=(bind "DataContext/Home/CanLoadMore") Click="Home.LoadMore" Height=48 FontSize=16 TextHorizontalAlignment="Left" TextVerticalAlignment="Center" class="octopus-action octopus-link" "load more posts"))
        (Grid Name="Loadinghome" Visible=(bind "DataContext/Home/ShowProgress") IgnoreHitTest=true
          (ActivityIndicator Name="LoadinghomeIndicator" class="octopus-dim"))))
    (PivotItem Name="Feedconnect" Header="connect"
      (Grid
        (StackView Name="Timelineconnect" Spacing=12 Overflow="hidden scroll" JustifyContent="Start"
          (StackView Name="Guestconnect" Visible=(bind "DataContext/Reader/IsGuest") Spacing=16 JustifyContent="Start"
            (StackView Spacing=8 Padding="0 0 24 0" JustifyContent="Start"
              (TextBlock Text="likes, reposts and replies" FontSize=22 WordWrap=true class="octopus-muted" IgnoreHitTest=true)
              (TextBlock Text="Sign in to see who interacts with your posts and who follows you." FontSize=14 WordWrap=true class="octopus-dim" IgnoreHitTest=true))
            (Button Name="SignInconnect" Click="SignIn" Height=48 FontSize=15 FontWeight="Bold" TextHorizontalAlignment="Center" TextVerticalAlignment="Center" class="octopus-action octopus-primary" "Sign in")
            (Button Name="SignUpconnect" Click="SignUp" Height=48 FontSize=15 TextHorizontalAlignment="Left" TextVerticalAlignment="Center" class="octopus-action octopus-link" "Create an account"))
          (ItemsStack Name="ActivityItems" Spacing=12 JustifyContent="Start" ItemNamePrefix="Activity" ItemsSource=(bind "DataContext/Connect/Items") ItemTemplate="Octopus/Templates/Activity")
          (StackView Name="EmptyTimelineconnect" Visible=(bind "DataContext/Connect/ShowEmpty") Spacing=8 Padding="0 0 24 0" JustifyContent="Start"
            (TextBlock Text="nothing yet" FontSize=22 WordWrap=true class="octopus-muted" IgnoreHitTest=true)
            (TextBlock Text="When people like, repost or reply to your posts, or follow you, it shows up here." FontSize=14 WordWrap=true class="octopus-dim" IgnoreHitTest=true))
          (StackView Name="TimelineErrorconnect" Visible=(bind "DataContext/Connect/ShowError") Spacing=8 Padding="0 0 24 0" JustifyContent="Start"
            (TextBlock Text="can't load activity" FontSize=22 WordWrap=true class="octopus-muted" IgnoreHitTest=true)
            (TextBlock FontSize=14 WordWrap=true class="octopus-dim" IgnoreHitTest=true Text=(+ (bind "DataContext/Connect/ErrorMessage") " Use refresh to try again."))))
        (Grid Name="Loadingconnect" Visible=(bind "DataContext/Connect/ShowProgress") IgnoreHitTest=true
          (ActivityIndicator Name="LoadingconnectIndicator" class="octopus-dim"))))
    (PivotItem Name="Feeddiscover" Header="discover"
      (Grid
        (StackView Name="Timelinediscover" Spacing=12 Overflow="hidden scroll" JustifyContent="Start"
          (ItemsStack Name="PostItems" Spacing=12 JustifyContent="Start" ItemNamePrefix="Post" ItemsSource=(bind "DataContext/Discover/Items") ItemTemplate="Octopus/Templates/PostCard")
          (StackView Name="EmptyTimelinediscover" Visible=(bind "DataContext/Discover/ShowEmpty") Spacing=8 Padding="0 0 24 0" JustifyContent="Start"
            (TextBlock Text="no posts yet" FontSize=22 WordWrap=true class="octopus-muted" IgnoreHitTest=true)
            (TextBlock Text="Be the first to write something." FontSize=14 WordWrap=true class="octopus-dim" IgnoreHitTest=true))
          (StackView Name="TimelineErrordiscover" Visible=(bind "DataContext/Discover/ShowError") Spacing=8 Padding="0 0 24 0" JustifyContent="Start"
            (TextBlock Text="can't load posts" FontSize=22 WordWrap=true class="octopus-muted" IgnoreHitTest=true)
            (TextBlock FontSize=14 WordWrap=true class="octopus-dim" IgnoreHitTest=true Text=(+ (bind "DataContext/Discover/ErrorMessage") " Use refresh to try again.")))
          (Grid Name="LoadingMorediscover" Visible=(bind "DataContext/Discover/IsLoadingMore") Height=64 IgnoreHitTest=true
            (ActivityIndicator class="octopus-dim"))
          (Button Name="LoadMorediscover" Visible=(bind "DataContext/Discover/CanLoadMore") Click="Discover.LoadMore" Height=48 FontSize=16 TextHorizontalAlignment="Left" TextVerticalAlignment="Center" class="octopus-action octopus-link" "load more posts"))
        (Grid Name="Loadingdiscover" Visible=(bind "DataContext/Discover/ShowProgress") IgnoreHitTest=true
          (ActivityIndicator Name="LoadingdiscoverIndicator" class="octopus-dim")))))
  (Grid Name="HomeStatusBar" Rows="20px" SafeAreaEdge="Bottom" Visible=false class="frosted"
    (TextBlock Name="HomeStatus" FontSize=11 TextHorizontalAlignment="Center" class="octopus-dim" IgnoreHitTest=true)))
