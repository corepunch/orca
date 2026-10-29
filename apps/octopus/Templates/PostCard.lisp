(Grid Columns="44px 1fr" Spacing=8 class="post-card"
  (Button Name="Avatar" Width=44 Height=44 MarginTop=(bind "DataContext/AvatarTop") Click="OpenAuthor" BackgroundColor="#00000000" class="octopus-row"
    (Grid Width=44 Height=44 StateClasses=(bind "DataContext/Post/Tone") IgnoreHitTest=true
      (TextBlock Text=(bind "DataContext/Post/Initials") FontSize=19 class="avatar-initials" TextHorizontalAlignment="Center" TextVerticalAlignment="Center" IgnoreHitTest=true)
      (ImageView Name="AvatarPhoto" Visible=(bind "DataContext/Post/HasAvatar") Src=(bind "DataContext/Post/AvatarPath") ForegroundColor="#ffffff" Stretch="UniformToFill" IgnoreHitTest=true)))
  (StackView Spacing=4
    (Button Name="RepostedBy" Visible=(bind "DataContext/IsRepost") Height=20 Click="OpenReposter" BackgroundColor="#00000000" class="octopus-row"
      (StackView Direction="Horizontal" AlignItems="Center" Spacing=4 IgnoreHitTest=true
        (VectorView Source="Octopus/assets/icons/repeat-2.svg" Width=14 Height=14 MarginTop="auto" MarginBottom="auto" class="octopus-dim" IgnoreHitTest=true)
        (TextBlock Text=(bind "DataContext/RepostedText") FontSize=13 class="octopus-dim" TextOverflow="Ellipsis" TextWrapping="NoWrap" IgnoreHitTest=true)))
    (Button Name="Author" Height=24 Click="OpenAuthor" BackgroundColor="#00000000" class="octopus-row"
      (Grid Columns="1fr auto" Spacing=8 IgnoreHitTest=true
        (TextBlock Name="PostAuthorName" Text=(bind "DataContext/Post/AuthorName") FontSize=16 FontWeight="Bold" TextOverflow="Ellipsis" TextWrapping="NoWrap" class="octopus-ink" IgnoreHitTest=true)
        (TextBlock Name="PostTimestamp" Text=(bind "DataContext/Post/Age") FontSize=16 TextWrapping="NoWrap" class="octopus-dim" IgnoreHitTest=true)))
    (Button Name="Open" Click="OpenPost" BackgroundColor="#00000000" class="octopus-row"
      (StackView Spacing=8 IgnoreHitTest=true
        (Grid Name="Photo" Visible=(bind "DataContext/Post/HasPhoto") AspectRatio=(bind "DataContext/Post/AspectRatio") class="photo-frame" StateClasses=(bind "DataContext/Post/PhotoFrame") IgnoreHitTest=true
          (VectorView Source="Octopus/assets/icons/image.svg" Visible=(bind "DataContext/Post/PhotoPending") Width=28 Height=28 MarginLeft="auto" MarginRight="auto" MarginTop="auto" MarginBottom="auto" class="octopus-dim" IgnoreHitTest=true)
          (ImageView Name="Image" Visible=(bind "DataContext/Post/PhotoShown") Src=(bind "DataContext/Post/PhotoPath") ForegroundColor="#ffffff" Stretch="UniformToFill" IgnoreHitTest=true)
          (ActivityIndicator Name="PhotoLoading" Visible=(bind "DataContext/Post/PhotoLoading") class="octopus-dim")
          (Button Name="RetryPhoto" Visible=(bind "DataContext/Post/PhotoFailed") Click="Photos.Retry" BackgroundColor="#00000000" class="octopus-row"
            (VectorView Name="PhotoUnavailable" Source="Octopus/assets/icons/image-off.svg" Width=28 Height=28 MarginLeft="auto" MarginRight="auto" MarginTop="auto" MarginBottom="auto" class="octopus-dim" IgnoreHitTest=true)))
        (TextBlock Name="PostBodyCaption" Visible=(bind "DataContext/Post/HasCaption") Text=(bind "DataContext/Post/Excerpt") FontSize=15 WordWrap=true class="octopus-ink" IgnoreHitTest=true)
        (Grid Visible=(bind "DataContext/Post/IsQuote") Columns="4px 1fr" Spacing=12 IgnoreHitTest=true
          (Grid class="octopus-accent-fill" IgnoreHitTest=true)
          (StackView Spacing=4 IgnoreHitTest=true
            (TextBlock Name="PostBodyQuote" Text=(bind "DataContext/Post/Quote") TrimLeading=true WordWrap=true class="post-quote octopus-ink" IgnoreHitTest=true)
            (TextBlock Name="PostBodyQuoteSource" Visible=(bind "DataContext/Post/HasQuoteSource") Text=(bind "DataContext/Post/QuoteSourceText") FontSize=14 WordWrap=true class="octopus-muted" IgnoreHitTest=true)))
        (TextBlock Name="PostBodyThoughts" Visible=(bind "DataContext/Post/HasThoughts") Text=(bind "DataContext/Post/Thoughts") FontSize=15 WordWrap=true class="octopus-ink" IgnoreHitTest=true)
        (Grid Visible=(bind "DataContext/Post/IsLink") Columns="44px 1fr" Spacing=8 IgnoreHitTest=true
          (Grid Name="PostBodyLinkTile" Width=44 Height=44 class="octopus-accent-fill" IgnoreHitTest=true
            (VectorView Source="Octopus/assets/icons/link.svg" Width=20 Height=20 MarginLeft="auto" MarginRight="auto" MarginTop="auto" MarginBottom="auto" class="octopus-on-accent" IgnoreHitTest=true))
          (StackView Spacing=4 IgnoreHitTest=true
            (TextBlock Name="PostBodyLinkTitle" Text=(bind "DataContext/Post/LinkTitle") TrimLeading=true WordWrap=true class="post-link-title octopus-accent-text" IgnoreHitTest=true)
            (TextBlock Name="PostBodyDomain" Text=(bind "DataContext/Post/Domain") FontSize=13 TextOverflow="Ellipsis" TextWrapping="NoWrap" class="octopus-dim" IgnoreHitTest=true)))
        (TextBlock Name="PostBodyTitle" Visible=(bind "DataContext/Post/IsText") Text=(bind "DataContext/Post/Title") TrimLeading=true WordWrap=true class="post-title octopus-ink" IgnoreHitTest=true)
        (TextBlock Name="PostBodyContent" Visible=(bind "DataContext/Post/HasContent") Text=(bind "DataContext/Post/Excerpt") WordWrap=true class="post-content" IgnoreHitTest=true)))
    (ItemsStack Name="Tags" Visible=(bind "DataContext/Post/HasTags") Direction="Horizontal" AlignItems="Center" Spacing=8 Height=28 ItemNamePrefix="Tag" ItemsSource=(bind "DataContext/Post/Tags") ItemTemplate="Octopus/Templates/Tag")
    (Grid Name="Actions" Columns="1fr 1fr 1fr" Height=44 StateClasses=(bind "DataContext/Post/State")
      (Button Name="Comment" Height=44 Click="OpenPost" BackgroundColor="#00000000" class="octopus-row"
        (StackView Direction="Horizontal" AlignItems="Center" Spacing=8 Height=28 MarginBottom="auto" IgnoreHitTest=true
          (VectorView Source="Octopus/assets/icons/message-circle.svg" Width=20 Height=20 MarginTop="auto" MarginBottom="auto" class="reaction" IgnoreHitTest=true)
          (TextBlock Text=(bind "DataContext/Post/CommentsText") FontSize=13 class="reaction" IgnoreHitTest=true)))
      (Button Name="Repost" Height=44 Click="Repost" IgnoreHitTest=(bind "DataContext/Post/Locked") BackgroundColor="#00000000" class="octopus-row reaction-button"
        (StackView Direction="Horizontal" AlignItems="Center" Spacing=8 Height=28 MarginBottom="auto" IgnoreHitTest=true
          (VectorView Source="Octopus/assets/icons/repeat-2.svg" Width=20 Height=20 MarginTop="auto" MarginBottom="auto" class="reaction repost-reaction" IgnoreHitTest=true)
          (TextBlock Text=(bind "DataContext/Post/RepostsText") FontSize=13 class="reaction repost-reaction" IgnoreHitTest=true)))
      (Button Name="Like" Height=44 Click="Like" IgnoreHitTest=(bind "DataContext/Post/Locked") BackgroundColor="#00000000" class="octopus-row reaction-button"
        (StackView Direction="Horizontal" AlignItems="Center" Spacing=8 Height=28 MarginBottom="auto" IgnoreHitTest=true
          (VectorView Source="Octopus/assets/icons/heart.svg" Width=20 Height=20 MarginTop="auto" MarginBottom="auto" class="reaction like-reaction" IgnoreHitTest=true)
          (TextBlock Text=(bind "DataContext/Post/LikesText") FontSize=13 class="reaction like-reaction" IgnoreHitTest=true))))))
