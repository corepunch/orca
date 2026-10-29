(StackView Name="ProfilePage" Spacing=18 Overflow="hidden scroll" JustifyContent="Start"
  (Grid Columns="96px 1fr" Spacing=16
    (Grid Name="ProfileAvatar" Width=96 Height=96 StateClasses=(bind "DataContext/Profile/Tone")
      (TextBlock Text=(bind "DataContext/Profile/Initials") FontSize=34 class="avatar-initials" TextHorizontalAlignment="Center" TextVerticalAlignment="Center" IgnoreHitTest=true)
      (ImageView Name="AvatarPhoto" Visible=(bind "DataContext/Profile/HasAvatar") Src=(bind "DataContext/Profile/AvatarPath") ForegroundColor="#ffffff" Stretch="UniformToFill" IgnoreHitTest=true))
    (StackView Spacing=6 JustifyContent="Center"
      (TextBlock Name="ProfileName" Text=(bind "DataContext/Profile/Title") FontSize=26 WordWrap=true class="octopus-ink" IgnoreHitTest=true)
      (TextBlock Name="ProfileBio" Text=(bind "DataContext/Profile/BioText") FontSize=14 WordWrap=true class="octopus-muted" IgnoreHitTest=true)))
  (Grid Name="ProfileCounts" Columns="1fr 1fr 1fr" Height=64
    (StackView Spacing=0 JustifyContent="Start"
      (TextBlock Name="Countposts" Text=(bind "DataContext/Profile/PostsText") FontSize=30 class="octopus-ink" IgnoreHitTest=true)
      (TextBlock Text="posts" FontSize=13 class="octopus-dim" IgnoreHitTest=true))
    (StackView Spacing=0 JustifyContent="Start"
      (TextBlock Name="Countfollowers" Text=(bind "DataContext/Profile/FollowersText") FontSize=30 class="octopus-ink" IgnoreHitTest=true)
      (TextBlock Text="followers" FontSize=13 class="octopus-dim" IgnoreHitTest=true))
    (StackView Spacing=0 JustifyContent="Start"
      (TextBlock Name="Countfollowing" Text=(bind "DataContext/Profile/FollowingText") FontSize=30 class="octopus-ink" IgnoreHitTest=true)
      (TextBlock Text="following" FontSize=13 class="octopus-dim" IgnoreHitTest=true)))
  (StackView Name="ProfileActions" Spacing=12 JustifyContent="Start"
    (Button Name="SignInToFollow" Visible=(bind "DataContext/Reader/IsGuest") Click="SignInToFollow" Height=48 FontSize=15 FontWeight="Bold" TextHorizontalAlignment="Center" TextVerticalAlignment="Center" class="octopus-action octopus-primary" "Sign in to follow")
    (Button Name="FollowButton" Visible=(bind "DataContext/Profile/CanFollow") Click="Follow" Height=44 BorderWidth=2 FontSize=15 FontWeight="Bold" TextHorizontalAlignment="Center" TextVerticalAlignment="Center" Text=(bind "DataContext/Profile/FollowText") StateClasses=(bind "DataContext/Profile/FollowStyle") class="octopus-action"))
  (TextBlock Text="posts" FontSize=20 MarginTop=8 class="octopus-accent-text" IgnoreHitTest=true)
  (StackView Name="ProfilePosts" Spacing=12 JustifyContent="Start"
    (ItemsStack Name="PostItems" Spacing=12 JustifyContent="Start" ItemNamePrefix="Post" ItemsSource=(bind "DataContext/ProfilePosts/Items") ItemTemplate="Octopus/Templates/PostCard")
    (Grid Name="ProfileLoading" Visible=(bind "DataContext/ProfilePosts/ShowLoading") Height=160 IgnoreHitTest=true
      (ActivityIndicator Name="ProfileLoadingIndicator" class="octopus-dim"))
    (StackView Name="NoProfilePosts" Visible=(bind "DataContext/ProfilePosts/ShowEmpty") Spacing=8 Padding="0 0 24 0" JustifyContent="Start"
      (TextBlock Text="no posts yet" FontSize=22 WordWrap=true class="octopus-muted" IgnoreHitTest=true)
      (TextBlock Visible=(bind "DataContext/Profile/Own") Text="Use write to share your first post." FontSize=14 WordWrap=true class="octopus-dim" IgnoreHitTest=true))
    (StackView Name="ProfilePostsError" Visible=(bind "DataContext/ProfilePosts/ShowError") Spacing=8 Padding="0 0 24 0" JustifyContent="Start"
      (TextBlock Text="can't load posts" FontSize=22 WordWrap=true class="octopus-muted" IgnoreHitTest=true)
      (TextBlock Text=(bind "DataContext/ProfilePosts/ErrorMessage") FontSize=14 WordWrap=true class="octopus-dim" IgnoreHitTest=true))))
