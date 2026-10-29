(Node2D Name="PostRoot"
  (StackView Name="PostPage" PaddingTop=20 Spacing=16 Overflow="hidden scroll" JustifyContent="Start"
    (StackView Name="PostDetail" Spacing=16 JustifyContent="Start" class="post-detail"
      (Button Name="PostAuthor" Height=60 Click="OpenAuthor" BackgroundColor="#00000000" class="octopus-row"
        (Grid Columns="56px 1fr" Spacing=14 IgnoreHitTest=true
          (Grid Width=56 Height=56 StateClasses=(bind "DataContext/Tone") IgnoreHitTest=true
            (TextBlock Text=(bind "DataContext/Initials") FontSize=19 class="avatar-initials" TextHorizontalAlignment="Center" TextVerticalAlignment="Center" IgnoreHitTest=true)
            (ImageView Name="AvatarPhoto" Visible=(bind "DataContext/HasAvatar") Src=(bind "DataContext/AvatarPath") ForegroundColor="#ffffff" Stretch="UniformToFill" IgnoreHitTest=true))
          (StackView Spacing=2 JustifyContent="Center" IgnoreHitTest=true
            (TextBlock Name="PostAuthorName" Text=(bind "DataContext/AuthorName") FontSize=22 TextOverflow="Ellipsis" TextWrapping="NoWrap" class="octopus-ink" IgnoreHitTest=true)
            (TextBlock Name="PostKind" FontSize=13 class="octopus-dim" IgnoreHitTest=true Text=(+ (+ (bind "DataContext/KindLabel") " · ") (bind "DataContext/Age"))))))
      (StackView Name="PostBody" Spacing=8 IgnoreHitTest=true
        (Grid Name="Photo" Visible=(bind "DataContext/HasPhoto") AspectRatio=(bind "DataContext/AspectRatio") class="photo-frame" StateClasses=(bind "DataContext/PhotoFrame")
          (VectorView Source="Octopus/assets/icons/image.svg" Visible=(bind "DataContext/PhotoPending") Width=28 Height=28 MarginLeft="auto" MarginRight="auto" MarginTop="auto" MarginBottom="auto" class="octopus-dim" IgnoreHitTest=true)
          (ImageView Name="Image" Visible=(bind "DataContext/PhotoShown") Src=(bind "DataContext/PhotoPath") ForegroundColor="#ffffff" Stretch="UniformToFill" IgnoreHitTest=true)
          (ActivityIndicator Name="PhotoLoading" Visible=(bind "DataContext/PhotoLoading") class="octopus-dim")
          (Button Name="RetryPhoto" Visible=(bind "DataContext/PhotoFailed") Click="Photos.Retry" BackgroundColor="#00000000" class="octopus-row"
            (VectorView Name="PhotoUnavailable" Source="Octopus/assets/icons/image-off.svg" Width=28 Height=28 MarginLeft="auto" MarginRight="auto" MarginTop="auto" MarginBottom="auto" class="octopus-dim" IgnoreHitTest=true)))
        (TextBlock Name="PostBodyCaption" Visible=(bind "DataContext/HasCaption") Text=(bind "DataContext/Body") FontSize=15 WordWrap=true class="octopus-ink" IgnoreHitTest=true)
        (Grid Visible=(bind "DataContext/IsQuote") Columns="4px 1fr" Spacing=12 IgnoreHitTest=true
          (Grid class="octopus-accent-fill" IgnoreHitTest=true)
          (StackView Spacing=4 IgnoreHitTest=true
            (TextBlock Name="PostBodyQuote" Text=(bind "DataContext/Quote") TrimLeading=true WordWrap=true class="post-quote octopus-ink" IgnoreHitTest=true)
            (TextBlock Name="PostBodyQuoteSource" Visible=(bind "DataContext/HasQuoteSource") Text=(bind "DataContext/QuoteSourceText") FontSize=14 WordWrap=true class="octopus-muted" IgnoreHitTest=true)))
        (TextBlock Name="PostBodyThoughts" Visible=(bind "DataContext/HasThoughts") Text=(bind "DataContext/Thoughts") FontSize=15 WordWrap=true class="octopus-ink" IgnoreHitTest=true)
        (Grid Visible=(bind "DataContext/IsLink") Columns="44px 1fr" Spacing=8 IgnoreHitTest=true
          (Grid Name="PostBodyLinkTile" Width=44 Height=44 class="octopus-accent-fill" IgnoreHitTest=true
            (VectorView Source="Octopus/assets/icons/link.svg" Width=20 Height=20 MarginLeft="auto" MarginRight="auto" MarginTop="auto" MarginBottom="auto" class="octopus-on-accent" IgnoreHitTest=true))
          (StackView Spacing=4 IgnoreHitTest=true
            (TextBlock Name="PostBodyLinkTitle" Text=(bind "DataContext/LinkTitle") TrimLeading=true WordWrap=true class="post-link-title octopus-accent-text" IgnoreHitTest=true)
            (TextBlock Name="PostBodyDomain" Text=(bind "DataContext/Domain") FontSize=13 TextOverflow="Ellipsis" TextWrapping="NoWrap" class="octopus-dim" IgnoreHitTest=true)))
        (TextBlock Name="PostBodyTitle" Visible=(bind "DataContext/IsText") Text=(bind "DataContext/Title") TrimLeading=true WordWrap=true class="post-title octopus-ink" IgnoreHitTest=true)
        (TextBlock Name="PostBodyContent" Visible=(bind "DataContext/HasContent") Text=(bind "DataContext/Body") WordWrap=true class="post-content" IgnoreHitTest=true))
      (ItemsStack Name="Tags" Visible=(bind "DataContext/HasTags") Direction="Horizontal" AlignItems="Center" Spacing=8 Height=28 ItemNamePrefix="Tag" ItemsSource=(bind "DataContext/Tags") ItemTemplate="Octopus/Templates/Tag")
      (Button Name="CopyLink" Visible=(bind "DataContext/IsLink") Click="CopyLink" Height=44 BorderWidth=2 FontSize=15 FontWeight="Bold" TextHorizontalAlignment="Center" TextVerticalAlignment="Center" class="octopus-action octopus-outline" "Copy link")
      (TextBlock Name="PostTime" Text=(bind "DataContext/Stamp") FontSize=13 class="octopus-dim" IgnoreHitTest=true)
      (StackView Spacing=6 JustifyContent="Start"
        (Grid Height=1 class="octopus-rule" IgnoreHitTest=true)
        (Grid Name="PostStats" Columns="1fr 1fr 1fr" Height=64
          (StackView Spacing=0 JustifyContent="Center"
            (TextBlock Name="Statlikes" Text=(bind "DataContext/LikeCount") FontSize=28 class="octopus-ink" IgnoreHitTest=true)
            (TextBlock Text=(bind "DataContext/LikesLabel") FontSize=13 class="octopus-dim" IgnoreHitTest=true))
          (StackView Spacing=0 JustifyContent="Center"
            (TextBlock Name="Statreposts" Text=(bind "DataContext/RepostCount") FontSize=28 class="octopus-ink" IgnoreHitTest=true)
            (TextBlock Text=(bind "DataContext/RepostsLabel") FontSize=13 class="octopus-dim" IgnoreHitTest=true))
          (StackView Spacing=0 JustifyContent="Center"
            (TextBlock Name="Statcomments" Text=(bind "DataContext/CommentCount") FontSize=28 class="octopus-ink" IgnoreHitTest=true)
            (TextBlock Text=(bind "DataContext/CommentsLabel") FontSize=13 class="octopus-dim" IgnoreHitTest=true)))
        (Grid Height=1 class="octopus-rule" IgnoreHitTest=true)))
    (TextBlock Text="comments" FontSize=20 MarginTop=8 class="octopus-accent-text" IgnoreHitTest=true)
    (StackView Name="Comments" Spacing=18 JustifyContent="Start"
      (ItemsStack Name="CommentItems" Spacing=18 JustifyContent="Start" ItemNamePrefix="Comment" ItemsSource=(bind "DataContext/Comments/Items") ItemTemplate="Octopus/Templates/Comment")
      (Grid Name="CommentsLoading" Visible=(bind "DataContext/Comments/IsBusy") Height=64 IgnoreHitTest=true
        (ActivityIndicator Name="CommentsLoadingIndicator" class="octopus-dim"))
      (StackView Name="NoComments" Visible=(bind "DataContext/Comments/ShowEmpty") Spacing=8 Padding="0 0 24 0" JustifyContent="Start"
        (TextBlock Text="no comments yet" FontSize=22 WordWrap=true class="octopus-muted" IgnoreHitTest=true)
        (TextBlock FontSize=14 WordWrap=true class="octopus-dim" IgnoreHitTest=true Text=(if (bind "DataContext/Reader/IsGuest") "Sign in to start the conversation." "Start the conversation below.")))
      (StackView Name="CommentsFailed" Visible=(bind "DataContext/Comments/ShowError") Spacing=8 Padding="0 0 24 0" JustifyContent="Start"
        (TextBlock Text="can't load comments" FontSize=22 WordWrap=true class="octopus-muted" IgnoreHitTest=true)
        (TextBlock Text=(bind "DataContext/Comments/ErrorMessage") FontSize=14 WordWrap=true class="octopus-dim" IgnoreHitTest=true))))
  (Node2D Name="CommentBar" SafeAreaEdge="Bottom" MarginLeft=-20 MarginRight=-20 Padding="20 8" class="frosted bar-top"
    (StackView Spacing=6 JustifyContent="Start"
      (TextBlock Name="PostStatus" Visible=false FontSize=11 TextHorizontalAlignment="Center" class="octopus-dim")
      (Grid Name="ReplyBar" Visible=(bind "DataContext/Draft/Replying") Columns="1fr 36px" Height=32
        (TextBlock Name="ReplyLabel" Text=(bind "DataContext/Draft/ReplyLabel") FontSize=13 TextVerticalAlignment="Center" class="octopus-accent-text" IgnoreHitTest=true)
        (Button Name="CancelReply" Click="CancelReply" Width=36 Height=32 BackgroundColor="#00000000" class="octopus-row"
          (VectorView Source="Octopus/assets/icons/x.svg" Width=16 Height=16 MarginLeft="auto" MarginRight="auto" MarginTop="auto" MarginBottom="auto" class="octopus-muted" IgnoreHitTest=true)))
      (Input Name="CommentInput" Visible=(bind "DataContext/Reader/SignedIn") PlaceholderText="Write a comment" Height=44 FontSize=15 Padding="12 0" BorderWidth=2 Submit="Draft.Submit" class="octopus-input octopus-field" Text=(bind "DataContext/Draft/Text" Mode=TwoWay))
      (Button Name="SignInToComment" Visible=(bind "DataContext/Reader/IsGuest") Click="SignInToComment" Height=40 FontSize=15 TextHorizontalAlignment="Left" TextVerticalAlignment="Center" class="octopus-action octopus-link" "Sign in to comment"))))
